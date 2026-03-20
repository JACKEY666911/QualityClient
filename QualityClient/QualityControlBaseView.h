#ifndef QUALITYCONTROLBASEVIEW_H
#define QUALITYCONTROLBASEVIEW_H

#include <QWidget>
#include <QList>
#include <QSet>
#include <QHash>

#include "widgets/AnnotationGraphicsView.h"

class QStackedWidget;
class ThumbnailWidget;
class CollapsiblePanel;
class LayerViewWidget;
class XrayImageButton;
class QualityControlShortcutManager;
class QLabel;
class QPushButton;

class QualityControlBaseView : public QWidget
{
    Q_OBJECT
public:
    explicit QualityControlBaseView(QWidget *parent = nullptr);

    void setBrand(const QString &brandKey);
    void setMainImage(const QPixmap &pixmap);
    void setAuxImage(const QPixmap &pixmap);
    void setAvailableXrayTypes(const QSet<int> &types);
    void setXrayTypeAvailable(int type, bool available);
    void setXrayTypeSelected(int type);
    void clearXraySelection();
    QList<AnnotationGraphicsView::AnnotationData> mainAnnotations() const;
    QList<AnnotationGraphicsView::AnnotationData> auxAnnotations() const;
    void setJudgeResultText(const QString &text);
    void setFreshnessText(const QString &text);
    void setChannelText(const QString &text);
    void setTimeText(const QString &text);

signals:
    void xrayTypeUnavailableClicked(int type);
    void xrayTypeToggled(int type, bool selected);

protected slots:
    void handleViewActivated(AnnotationGraphicsView *view);

protected:
    struct XrayButtonSpec {
        int typeId;
        QString label;
        QString unavailableImage;
        QString availableImage;
        QString selectedImage;
    };

    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void changeEvent(QEvent *event) override;

    void initializeLayout();
    virtual QWidget *buildTopBar() = 0;
    QWidget *buildMiddleArea();
    QWidget *buildBottomBar();
    QWidget *buildBrandButtons(const QList<XrayButtonSpec> &specs, QList<XrayImageButton*> *buttonList);
    void positionOverlayPanel();
    void updateOverlayGeometry();
    void handleXrayButtonClicked(XrayImageButton *button);
    static QList<XrayButtonSpec> tongfangSpecs();
    static QList<XrayButtonSpec> yisuoSpecs();
    void setShortcutsEnabled(bool enabled);

    AnnotationGraphicsView *m_mainView;
    AnnotationGraphicsView *m_auxView;
    AnnotationGraphicsView *m_activeView;

    QStackedWidget *m_brandStack;
    ThumbnailWidget *m_thumbnail;
    QWidget *m_middleArea;
    CollapsiblePanel *m_overlayPanel;
    LayerViewWidget *m_layerView;
    QHash<int, XrayImageButton*> m_xrayButtons;
    QList<XrayImageButton*> m_tongfangButtons;
    QList<XrayImageButton*> m_yisuoButtons;
    int m_selectedXrayType;

    QLabel *m_timeValueLabel;
    QLabel *m_judgeValueLabel;
    QLabel *m_freshnessValueLabel;
    QLabel *m_channelValueLabel;
    QPushButton *m_startCheckButton;
    QPushButton *m_passButton;
    QPushButton *m_detailButton;
    QPushButton *m_aiToggleButton;
    QPushButton *m_escButton;
    QPushButton *m_zoomInButton;
    QPushButton *m_zoomOutButton;

    QualityControlShortcutManager *m_shortcutManager;
    bool m_isYisuoBrand;
    bool m_shortcutsEnabled;
};

#endif // QUALITYCONTROLBASEVIEW_H
