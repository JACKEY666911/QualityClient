#include "LayerViewModel.h"
#include "XrayImage.h"

LayerViewModel::LayerViewModel(QObject *parent)
    : QObject(parent),
      m_isModify(false),
      m_qualityResult(-1),
      m_analysisDuration(-1),
      m_selectedXrayItem(nullptr),
      m_currentStartIndex(0)
{
}

QList<XrayImage*> LayerViewModel::xrayImageLayerItems() const
{
    return m_xrayImageLayerItems;
}

void LayerViewModel::setXrayImageLayerItems(const QList<XrayImage*> &items)
{
    m_xrayImageLayerItems = items;
    m_currentStartIndex = 0;
    updateDisplayedItems();
}

QList<XrayImage*> LayerViewModel::displayedLayerItems() const
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

XrayImage *LayerViewModel::selectedXrayItem() const
{
    return m_selectedXrayItem;
}

void LayerViewModel::setSelectedXrayItem(XrayImage *item)
{
    if (m_selectedXrayItem == item) {
        return;
    }
    m_selectedXrayItem = item;
    emit selectionChanged(item);
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

void LayerViewModel::selectLayerItem(XrayImage *item)
{
    if (!item || m_selectedXrayItem == item) {
        return;
    }
    for (XrayImage *img : m_xrayImageLayerItems) {
        img->setSelected(img == item);
    }
    setSelectedXrayItem(item);
}

void LayerViewModel::selectDisplayedIndex(int index)
{
    if (index < 0 || index >= m_displayedLayerItems.size()) {
        return;
    }
    selectLayerItem(m_displayedLayerItems.at(index));
}

void LayerViewModel::selectFirstItem()
{
    if (!m_displayedLayerItems.isEmpty()) {
        selectLayerItem(m_displayedLayerItems.first());
    }
}

void LayerViewModel::clearSelect()
{
    m_selectedXrayItem = nullptr;
    for (XrayImage *img : m_xrayImageLayerItems) {
        img->setSelected(false);
    }
    m_isModify = false;
    emit selectionChanged(nullptr);
}

void LayerViewModel::clearXrayImageLayerItems()
{
    m_xrayImageLayerItems.clear();
    m_displayedLayerItems.clear();
    m_selectedXrayItem = nullptr;
    m_analysisDuration = -1;
    m_isModify = false;
    m_topViewImage = QPixmap();
    emit displayedItemsChanged();
    emit topViewImageChanged();
    emit durationChanged();
    emit selectionChanged(nullptr);
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
