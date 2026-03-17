#ifndef LAYERVIEWMODEL_H
#define LAYERVIEWMODEL_H

#include <QObject>
#include <QList>
#include <QPixmap>

class XrayImage;

class LayerViewModel : public QObject
{
    Q_OBJECT
public:
    explicit LayerViewModel(QObject *parent = nullptr);

    QList<XrayImage*> xrayImageLayerItems() const;
    void setXrayImageLayerItems(const QList<XrayImage*> &items);

    QList<XrayImage*> displayedLayerItems() const;

    bool isModify() const;
    void setIsModify(bool value);

    QPixmap topViewImage() const;
    void setTopViewImage(const QPixmap &pixmap);

    int qualityResult() const;
    void setQualityResult(int value);

    qint64 analysisDuration() const;
    void setAnalysisDuration(qint64 value);

    XrayImage *selectedXrayItem() const;
    void setSelectedXrayItem(XrayImage *item);

    int currentStartIndex() const;
    void setCurrentStartIndex(int index);

    bool isPrevButtonEnabled() const;
    bool isNextButtonEnabled() const;

    void prevPage();
    void nextPage();
    void selectLayerItem(XrayImage *item);
    void selectDisplayedIndex(int index);
    void selectFirstItem();
    void clearSelect();
    void clearXrayImageLayerItems();
    void updateDisplayedItems();

signals:
    void displayedItemsChanged();
    void selectionChanged(XrayImage *item);
    void topViewImageChanged();
    void durationChanged();
    void buttonsChanged();

private:
    int pageSize() const { return 3; }
    void notifyButtons();

    QList<XrayImage*> m_xrayImageLayerItems;
    QList<XrayImage*> m_displayedLayerItems;
    bool m_isModify;
    QPixmap m_topViewImage;
    int m_qualityResult;
    qint64 m_analysisDuration;
    XrayImage *m_selectedXrayItem;
    int m_currentStartIndex;
};

#endif // LAYERVIEWMODEL_H
