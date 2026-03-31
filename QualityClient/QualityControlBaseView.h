#ifndef QUALITYCONTROLBASEVIEW_H
#define QUALITYCONTROLBASEVIEW_H

#include <QWidget>
#include <QList>
#include <QSet>
#include <QHash>
#include <QStringList>
#include <functional>

#include "Models/GlobalEnums.h"
#include "Models/ImageDistributeInfo.h"
#include "widgets/AnnotationGraphicsView.h"

class QStackedWidget;
class ThumbnailWidget;
class CollapsiblePanel;
class LayerViewWidget;
class XrayImageButton;
class QualityControlShortcutManager;
class QLabel;
class QPushButton;
class QTimer;

class QualityControlBaseView : public QWidget
{
    Q_OBJECT
public:
    explicit QualityControlBaseView(QWidget *parent = nullptr);

    void setBrand(ServerType type);
    void setBrandByType(int type);
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
    void clearBaseViewState();
    void setBottomBarTaskActive(bool hasTask);

signals:
    void startCheckRequested();
    void passRequested();
    void detailRequested();
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

    void initializeLayout(QWidget *host = nullptr);
    virtual QWidget *buildTopBar() = 0;
    QWidget *buildMiddleArea();
    QWidget *buildBottomBar();
    QWidget *buildBrandButtons(ServerType brandType, const QList<XrayButtonSpec> &specs, QList<XrayImageButton*> *buttonList);
    void positionOverlayPanel();
    void updateOverlayGeometry();
    void handleXrayButtonClicked(XrayImageButton *button);
    static QList<XrayButtonSpec> tongfangSpecs();
    static QList<XrayButtonSpec> yisuoSpecs();
    void setShortcutsEnabled(bool enabled);
    void applySafeMode(bool enabled);
    void updateToolIconsByBrand();
    void configureTaskImages(const ImageDistributeInfo &info);
    void clearTaskImageState();
    bool selectTypeFromOverlayOrFallback(int type, bool syncOverlay, bool syncButtons, bool emitToggle);
    void applyOverlaySelection(int index, bool syncButtonSelection);
    void rebuildOverlayLayerCards();
    QPixmap resolveTaskPixmap(const QString &url) const;
    void preloadTaskImageUrls(const QStringList &urls);
    QString originMainUrl() const;
    QString originAssistUrl() const;
    static int imageTypeOf(const XrayImage &img);
    static QSet<int> imageTypesOf(const QList<XrayImage> &images);
    static QList<XrayImage> buildOverlayItems(const ImageDistributeInfo &info);
    static QStringList collectImageUrls(const XrayImage &img);
    void onImageProviderUpdated(const QString &url, bool success);
    int originOverlayIndex() const;
    bool isOriginImageReady(const QString &url) const;
    bool isOriginPairReady() const;
    void startOriginGate(int initialIndex);
    void finishOriginGate(bool timeoutFallback);
    void selectToolButtonsByBrand(ServerType brandType);
    bool isDisplayTargetReady(const QString &mainUrl, const QString &auxUrl) const;
    void clearPendingDisplayTarget();
    void queuePendingDisplayTarget(int type, int overlayIndex, const QString &mainUrl, const QString &auxUrl);
    bool tryApplyPendingDisplayTarget();
    template <typename TWidget>
    void showOverlayChild(TWidget *&child,
                          const std::function<TWidget*()> &creator,
                          const std::function<void(TWidget*)> &onCreated = {});
    template <typename TWidget>
    void hideOverlayChild(TWidget *child, const std::function<void(TWidget*)> &onHidden = {});
    template <typename TWidget>
    void syncOverlayChildGeometry(TWidget *child) const;
    QWidget *contentParent() const;

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
    QHash<int, QPushButton*> m_escButtonsByBrand;
    QHash<int, QPushButton*> m_zoomInButtonsByBrand;
    QHash<int, QPushButton*> m_zoomOutButtonsByBrand;

    QualityControlShortcutManager *m_shortcutManager;
    bool m_shortcutsEnabled;
    bool m_isSafeMode;
    bool m_bottomBarTaskActive;
    ServerType m_currentBrandType;

    QStackedWidget *m_mainStack;

    QList<XrayImage> m_overlayItems;               // 当前任务用于 Overlay 面板展示的图层列表（来源 pbXrayImages）
    QHash<int, int> m_overlayIndexByType;          // 图像类型 -> Overlay 下标映射，便于按钮点击快速定位
    QHash<int, XrayImage> m_fallbackImageByType;   // 图像类型 -> xrayImages 兜底图映射（Overlay 无该类型时使用）
    QSet<int> m_availableXrayTypes;                // 左下角按钮可用类型集合（当前规则：并集）
    QSet<int> m_intersectionXrayTypes;             // 按钮点击后允许触发图像联动的类型集合（当前规则：pb 中可映射到左下角按钮的类型）
    mutable QHash<QString, QPixmap> m_taskImageCache; // 当前任务级图片缓存（url -> pixmap）
    QSet<QString> m_taskPendingUrls;               // 当前任务正在下载中的 URL 集合（防重复请求）
    QSet<QString> m_taskDeferredUrls;              // 等待原图阶段暂存的延后预下载 URL（原图就绪后再发起）
    QString m_taskOriginMainUrl;                   // 当前任务原图主视角 URL（用于“原图优先”门控）
    QString m_taskOriginAssistUrl;                 // 当前任务原图辅视角 URL
    bool m_waitingOriginReady = false;             // 是否正在等待原图就绪后再显示
    bool m_originSubmitBlocked = false;            // 等待原图阶段是否拦截“开检/放行”
    QString m_currentMainUrl;                      // 当前主视图正在显示/目标显示的 URL
    QString m_currentAuxUrl;                       // 当前辅视图正在显示/目标显示的 URL
    int m_pendingType = -1;                        // 待生效的按钮类型（最后一次点击覆盖）
    int m_pendingOverlayIndex = -1;                // 待生效的 overlay 下标（-1 表示非 overlay 目标）
    QString m_pendingMainUrl;                      // 待生效主图 URL
    QString m_pendingAuxUrl;                       // 待生效辅图 URL
    int m_originInitialIndex = 0;                  // 当前任务原图门禁对应的初始 overlay 下标
    QTimer *m_originGateTimer = nullptr;           // 原图门禁超时定时器
    QWidget *m_layoutHost = nullptr;               // 主页内容宿主，默认是本体，可切到 stack page
};

template <typename TWidget>
void QualityControlBaseView::showOverlayChild(TWidget *&child,
                                              const std::function<TWidget*()> &creator,
                                              const std::function<void(TWidget*)> &onCreated)
{
    if (child && child->isVisible()) {
        return;
    }
    if (!child) {
        child = creator ? creator() : nullptr;
        if (child && onCreated) {
            onCreated(child);
        }
    }
    if (!child) {
        return;
    }
    child->setGeometry(rect());
    child->show();
    child->raise();
}

template <typename TWidget>
void QualityControlBaseView::hideOverlayChild(TWidget *child, const std::function<void(TWidget*)> &onHidden)
{
    if (!child) {
        return;
    }
    if (onHidden) {
        onHidden(child);
    }
    child->hide();
}

template <typename TWidget>
void QualityControlBaseView::syncOverlayChildGeometry(TWidget *child) const
{
    if (child) {
        child->setGeometry(rect());
    }
}

#endif // QUALITYCONTROLBASEVIEW_H
