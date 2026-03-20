#include "LayerViewModel.h"
#include "XrayImage.h"

LayerViewModel::LayerViewModel(QObject *parent)
    : QObject(parent),
      m_isModify(false),
      m_qualityResult(-1),
      m_analysisDuration(-1),
      m_selectedIndex(-1),
      m_currentStartIndex(0)
{
}

QList<XrayImage> LayerViewModel::xrayImageLayerItems() const
{
    return m_xrayImageLayerItems;
}

void LayerViewModel::setXrayImageLayerItems(const QList<XrayImage> &items)
{
    m_xrayImageLayerItems = items;
    m_currentStartIndex = 0;
    updateDisplayedItems();
}

QList<XrayImage> LayerViewModel::displayedLayerItems() const
{
    return m_displayedLayerItems;
}

bool LayerViewModel::isModify() const
{
    return m_isModify;
}

void LayerViewModel::setIsModify(bool value)
{
    if (m_isModify == value) {
        return;
    }
    m_isModify = value;
}

QPixmap LayerViewModel::topViewImage() const
{
    return m_topViewImage;
}

void LayerViewModel::setTopViewImage(const QPixmap &pixmap)
{
    m_topViewImage = pixmap;
    emit topViewImageChanged();
}

int LayerViewModel::qualityResult() const
{
    return m_qualityResult;
}

void LayerViewModel::setQualityResult(int value)
{
    m_qualityResult = value;
}

qint64 LayerViewModel::analysisDuration() const
{
    return m_analysisDuration;
}

void LayerViewModel::setAnalysisDuration(qint64 value)
{
    m_analysisDuration = value;
    emit durationChanged();
}

const XrayImage *LayerViewModel::selectedXrayItem() const
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_xrayImageLayerItems.size()) {
        return nullptr;
    }
    return &m_xrayImageLayerItems[m_selectedIndex];
}

void LayerViewModel::setSelectedXrayItem(int index)
{
    if (m_selectedIndex == index) {
        return;
    }
    m_selectedIndex = index;
    emit selectionChanged(index);
}

int LayerViewModel::currentStartIndex() const
{
    return m_currentStartIndex;
}

void LayerViewModel::setCurrentStartIndex(int index)
{
    if (m_currentStartIndex == index) {
        return;
    }
    m_currentStartIndex = index;
    updateDisplayedItems();
}

bool LayerViewModel::isPrevButtonEnabled() const
{
    return m_currentStartIndex > 0;
}

bool LayerViewModel::isNextButtonEnabled() const
{
    return m_currentStartIndex + pageSize() < m_xrayImageLayerItems.size();
}

void LayerViewModel::prevPage()
{
    if (m_currentStartIndex <= 0) {
        return;
    }
    m_currentStartIndex = qMax(m_currentStartIndex - pageSize(), 0);
    updateDisplayedItems();
}

void LayerViewModel::nextPage()
{
    if (m_currentStartIndex + pageSize() >= m_xrayImageLayerItems.size()) {
        return;
    }
    m_currentStartIndex = qMin(m_currentStartIndex + pageSize(), m_xrayImageLayerItems.size() - 1);
    updateDisplayedItems();
}

void LayerViewModel::selectLayerItem(int index)
{
    if (index < 0 || index >= m_xrayImageLayerItems.size()) {
        return;
    }
    for (int i = 0; i < m_xrayImageLayerItems.size(); ++i) {
        m_xrayImageLayerItems[i].setSelected(i == index);
    }
    setSelectedXrayItem(index);
}

void LayerViewModel::selectDisplayedIndex(int index)
{
    if (index < 0 || index >= m_displayedLayerItems.size()) {
        return;
    }
    const int absoluteIndex = m_currentStartIndex + index;
    if (absoluteIndex < 0 || absoluteIndex >= m_xrayImageLayerItems.size()) {
        return;
    }
    for (int i = 0; i < m_xrayImageLayerItems.size(); ++i) {
        m_xrayImageLayerItems[i].setSelected(i == absoluteIndex);
    }
    setSelectedXrayItem(absoluteIndex);
}

void LayerViewModel::selectFirstItem()
{
    if (!m_displayedLayerItems.isEmpty()) {
        selectDisplayedIndex(0);
    }
}

void LayerViewModel::clearSelect()
{
    m_selectedIndex = -1;
    for (int i = 0; i < m_xrayImageLayerItems.size(); ++i) {
        m_xrayImageLayerItems[i].setSelected(false);
    }
    m_isModify = false;
    emit selectionChanged(-1);
}

void LayerViewModel::clearXrayImageLayerItems()
{
    m_xrayImageLayerItems.clear();
    m_displayedLayerItems.clear();
    m_selectedIndex = -1;
    m_analysisDuration = -1;
    m_isModify = false;
    m_topViewImage = QPixmap();
    emit displayedItemsChanged();
    emit topViewImageChanged();
    emit durationChanged();
    emit selectionChanged(-1);
    notifyButtons();
}

void LayerViewModel::updateDisplayedItems()
{
    m_displayedLayerItems.clear();
    if (m_xrayImageLayerItems.isEmpty()) {
        notifyButtons();
        emit displayedItemsChanged();
        return;
    }
    int endIndex = qMin(m_currentStartIndex + pageSize(), m_xrayImageLayerItems.size());
    for (int i = m_currentStartIndex; i < endIndex; ++i) {
        m_displayedLayerItems.append(m_xrayImageLayerItems.at(i));
    }
    notifyButtons();
    emit displayedItemsChanged();
}

void LayerViewModel::notifyButtons()
{
    emit buttonsChanged();
}
