#ifndef LAYERVIEWWIDGET_H
#define LAYERVIEWWIDGET_H

#include <QWidget>
#include <QPixmap>

class QLabel;
class QPushButton;
class QHBoxLayout;
class QPixmap;
class XrayImage;

class LayerViewWidget : public QWidget
{
    Q_OBJECT
public:
    struct LayerItemData {
        QString name;
        QPixmap image;
        bool selected = false;
    };

    explicit LayerViewWidget(QWidget *parent = nullptr);

    void setTopImage(const QPixmap &pixmap);
    void setDurationText(const QString &text);
    void setLayerItems(const QList<LayerItemData> &items);
    void setXrayItems(const QList<XrayImage*> &items);
    void setPageSize(int size);
    int pageSize() const;
    int currentStartIndex() const;
    int selectedIndex() const;
    void setSelectedIndex(int index);

signals:
    void prevClicked();
    void nextClicked();
    void layerClicked(int index);
    void selectionChanged(int index);
    void pageChanged(int startIndex);
    void xrayItemSelected(XrayImage *item);

private:
    void rebuildLayerCards();
    void updateButtons();
    void prevPage();
    void nextPage();
    void selectIndexInternal(int index);
    void updateVisibleSelection();
    void ensureCardPool(int count);
    void updateVisibleCards();

    QLabel *m_topImageLabel;
    QLabel *m_durationLabel;
    QWidget *m_cardsContainer;
    QHBoxLayout *m_cardsLayout;
    QList<LayerItemData> m_layerItems;
    QList<XrayImage*> m_itemRefs;
    QPushButton *m_prevButton;
    QPushButton *m_nextButton;
    int m_pageSize;
    int m_currentStartIndex;
    int m_selectedIndex;
    QVector<QWidget*> m_visibleCards;
};

#endif // LAYERVIEWWIDGET_H
