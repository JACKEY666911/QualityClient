#include "QualityControlBaseView.h"

#include "widgets/AnnotationGraphicsView.h"
#include "widgets/CollapsiblePanel.h"
#include "widgets/LayerViewWidget.h"
#include "widgets/ThumbnailWidget.h"
#include "widgets/XrayImageButton.h"
#include "services/qualitycontrolshortcutmanager.h"
#include "services/imageprovider.h"
#include "Models/XImageType.h"
#include "Models/XrayImage.h"
#include "logging/logcategories.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QEvent>
#include <QPushButton>
#include <QPixmap>
#include <QIcon>
#include <QList>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QDateTime>
#include <QStringLiteral>
#include <QSizePolicy>
#include <QVariant>
#include <QUrl>
#include <QTimer>
#include <algorithm>

namespace {
bool isRemoteUrl(const QString &url)
{
    const QUrl parsed(url);
    const QString scheme = parsed.scheme().toLower();
    return parsed.isValid() && (scheme == QStringLiteral("http") || scheme == QStringLiteral("https"));
}

void appendUnique(QStringList &dst, const QStringList &src)
{
    for (const QString &item : src) {
        if (!item.isEmpty() && !dst.contains(item)) {
            dst.append(item);
        }
    }
}

ServerType normalizedServerType(int rawType)
{
    switch (rawType) {
    case ServerType::YiSuo:
        return ServerType::YiSuo;
    case ServerType::TongFang:
        return ServerType::TongFang;
    default:
        return ServerType::Unknown;
    }
}

bool isYiSuoServerType(ServerType type)
{
    return type == ServerType::YiSuo;
}

QString intSetToText(const QSet<int> &set)
{
    QList<int> values = set.values();
    std::sort(values.begin(), values.end());
    QStringList parts;
    parts.reserve(values.size());
    for (int v : values) {
        parts.append(QString::number(v));
    }
    return QStringLiteral("[%1]").arg(parts.join(QStringLiteral(",")));
}

QString overlayDurationText(const QVariant &durationValue)
{
    bool ok = false;
    const int seconds = durationValue.toInt(&ok);
    if (!ok || seconds <= 0) {
        return QStringLiteral("--");
    }
    return QStringLiteral("%1秒").arg(seconds);
}

constexpr int kOriginGateTimeoutMs = 10000;

struct ToolIconSet {
    const char *esc;
    const char *zoomIn;
    const char *zoomOut;
    const char *brandName;
};

ToolIconSet toolIconSetForServerType(ServerType type)
{
    switch (type) {
    case ServerType::YiSuo:
        return {":/Images/esc_origin.png", ":/Images/max.png", ":/Images/small.png", "YiSuo"};
    case ServerType::TongFang:
        return {":/Images/tf_esc.png", ":/Images/tf_big.png", ":/Images/tf_small.png", "TongFang"};
    case ServerType::HaiMan:
        // HaiMan 目前先复用同方图标，后续只需在这里替换资源路径。
        return {":/Images/tf_esc.png", ":/Images/tf_big.png", ":/Images/tf_small.png", "HaiMan(TF-Temp)"};
    default:
        return {":/Images/tf_esc.png", ":/Images/tf_big.png", ":/Images/tf_small.png", "Unknown(TF-Default)"};
    }
}

}

QualityControlBaseView::QualityControlBaseView(QWidget *parent)
    : QWidget(parent)
    , m_mainView(nullptr)
    , m_auxView(nullptr)
    , m_activeView(nullptr)
    , m_brandStack(nullptr)
    , m_thumbnail(nullptr)
    , m_middleArea(nullptr)
    , m_overlayPanel(nullptr)
    , m_layerView(nullptr)
    , m_selectedXrayType(-1)
    , m_timeValueLabel(nullptr)
    , m_judgeValueLabel(nullptr)
    , m_freshnessValueLabel(nullptr)
    , m_channelValueLabel(nullptr)
    , m_startCheckButton(nullptr)
    , m_passButton(nullptr)
    , m_detailButton(nullptr)
    , m_aiToggleButton(nullptr)
    , m_escButton(nullptr)
    , m_zoomInButton(nullptr)
    , m_zoomOutButton(nullptr)
    , m_shortcutManager(nullptr)
    , m_shortcutsEnabled(true)
    , m_isSafeMode(false)
    , m_bottomBarTaskActive(false)
    , m_currentBrandType(ServerType::Unknown)
{
    m_originGateTimer = new QTimer(this);
    m_originGateTimer->setSingleShot(true);
    connect(m_originGateTimer, &QTimer::timeout, this, [this]() {
        if (!m_waitingOriginReady) {
            return;
        }
        qCWarning(lcQcView) << "[BaseView] origin gate timeout, fallback enable submit actions"
                            << "taskOriginMain=" << m_taskOriginMainUrl
                            << "taskOriginAssist=" << m_taskOriginAssistUrl;
        finishOriginGate(true);
    });
    connect(&ImageProvider::instance(), &ImageProvider::imageUpdated,
            this, &QualityControlBaseView::onImageProviderUpdated);
}

void QualityControlBaseView::initializeLayout(QWidget *host)
{
    m_layoutHost = host ? host : this;
    QVBoxLayout *rootLayout = new QVBoxLayout;
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    rootLayout->addWidget(buildTopBar());
    rootLayout->addWidget(buildMiddleArea());
    rootLayout->addWidget(buildBottomBar());

    m_layoutHost->setLayout(rootLayout);
    m_layoutHost->setMinimumSize(1920, 1080);
    m_layoutHost->setMaximumSize(1920, 1080);
}

QList<QualityControlBaseView::XrayButtonSpec> QualityControlBaseView::tongfangSpecs()
{
    return {
        {XImage_TF_ED, QStringLiteral("ED"), QString(), QStringLiteral(":/Images/ED.png"), QStringLiteral(":/Images/ED_2.png")},
        {XImage_TF_GEN, QStringLiteral("GEN"), QString(), QStringLiteral(":/Images/GEN.png"), QStringLiteral(":/Images/GEN_2.png")},
        {XImage_TF_HI, QStringLiteral("HI"), QString(), QStringLiteral(":/Images/HI.png"), QStringLiteral(":/Images/HI_2.png")},
        {XImage_TF_LOW, QStringLiteral("LOW"), QString(), QStringLiteral(":/Images/LOW.png"), QStringLiteral(":/Images/LOW_2.png")},
        {XImage_TF_OS, QStringLiteral("OS"), QString(), QStringLiteral(":/Images/OS.png"), QStringLiteral(":/Images/OS_2.png")},
        {XImage_TF_MS, QStringLiteral("MS"), QString(), QStringLiteral(":/Images/MS.png"), QStringLiteral(":/Images/MS_2.png")}
    };
}

QList<QualityControlBaseView::XrayButtonSpec> QualityControlBaseView::yisuoSpecs()
{
    return {
        {XImage_YS_E0, QStringLiteral("E0"), QStringLiteral(":/Images/E0_gray.png"), QStringLiteral(":/Images/E0.png"), QStringLiteral(":/Images/E0_2.png")},
        {XImage_YS_E1, QStringLiteral("E1"), QStringLiteral(":/Images/E1_gray.png"), QStringLiteral(":/Images/E1.png"), QStringLiteral(":/Images/E1_2.png")},
        {XImage_YS_E2, QStringLiteral("E2"), QStringLiteral(":/Images/E2_gray.png"), QStringLiteral(":/Images/E2.png"), QStringLiteral(":/Images/E2_2.png")},
        {XImage_YS_SC, QStringLiteral("SC"), QStringLiteral(":/Images/SC_gray.png"), QStringLiteral(":/Images/SC.png"), QStringLiteral(":/Images/SC_2.png")},
        {XImage_YS_OS, QStringLiteral("OC"), QStringLiteral(":/Images/OC_gray.png"), QStringLiteral(":/Images/OC.png"), QStringLiteral(":/Images/OC_2.png")},
        {XImage_YS_HD, QStringLiteral("HD"), QStringLiteral(":/Images/HD_gray.png"), QStringLiteral(":/Images/HD.png"), QStringLiteral(":/Images/HD_2.png")}
    };
}

void QualityControlBaseView::setBrand(ServerType type)
{
    if (!m_brandStack) {
        return;
    }

    const ServerType normalized = normalizedServerType(static_cast<int>(type));
    m_currentBrandType = normalized;
    if (normalized == ServerType::Unknown) {
        qCWarning(lcQcView) << "[BaseView] illegal serverType, enter safe mode:" << static_cast<int>(type);
        applySafeMode(true);
        return;
    }

    applySafeMode(false);
    if (isYiSuoServerType(m_currentBrandType)) {
        m_brandStack->setCurrentIndex(1);
    } else {
        m_brandStack->setCurrentIndex(0);
    }
    selectToolButtonsByBrand(m_currentBrandType);
    // 任务先到、品牌后到时，需重新套用一次底栏状态，避免 brandStack 仍停留在 Unknown 禁用态。
    setBottomBarTaskActive(m_bottomBarTaskActive);
    qCInfo(lcQcView) << "[BaseView] setBrand serverType=" << static_cast<int>(m_currentBrandType);

    updateToolIconsByBrand();
    clearXraySelection();
    if (m_shortcutManager) {
        QHash<int, QAbstractButton*> typeButtons;
        for (auto it = m_xrayButtons.begin(); it != m_xrayButtons.end(); ++it) {
            typeButtons.insert(it.key(), it.value());
        }
        m_shortcutManager->setServerType(m_currentBrandType);
        m_shortcutManager->configure(typeButtons, m_escButton, m_startCheckButton, m_passButton, m_detailButton);
    }
}

void QualityControlBaseView::setBrandByType(int type)
{
    const ServerType normalized = normalizedServerType(type);
    if (normalized == ServerType::Unknown) {
        qCWarning(lcQcView) << "[BaseView] illegal server type from task:" << type;
    }
    setBrand(normalized);
}

void QualityControlBaseView::setMainImage(const QPixmap &pixmap)
{
    if (m_mainView) {
        m_mainView->setImage(pixmap);
    }
    if (m_thumbnail) {
        m_thumbnail->setImage(pixmap);
    }
    if (m_layerView) {
        m_layerView->setTopImage(pixmap);
    }
}

void QualityControlBaseView::setAuxImage(const QPixmap &pixmap)
{
    if (m_auxView) {
        m_auxView->setImage(pixmap);
    }
    if (m_thumbnail && m_activeView == m_auxView) {
        m_thumbnail->setImage(pixmap);
    }
}

void QualityControlBaseView::setAvailableXrayTypes(const QSet<int> &types)
{
    QSet<int> enabledTypes;
    QSet<int> disabledTypes;
    for (auto it = m_xrayButtons.begin(); it != m_xrayButtons.end(); ++it) {
        const bool available = types.contains(it.key());
        it.value()->setAvailable(available);
        if (available) {
            enabledTypes.insert(it.key());
        } else {
            disabledTypes.insert(it.key());
        }
        if (!available && it.value()->isSelected() && m_selectedXrayType == it.key()) {
            m_selectedXrayType = -1;
        }
    }
    qCInfo(lcQcView) << "[BaseView] xray buttons available="
                     << intSetToText(enabledTypes)
                     << "disabled=" << intSetToText(disabledTypes);
}

void QualityControlBaseView::setXrayTypeAvailable(int type, bool available)
{
    XrayImageButton *button = m_xrayButtons.value(type, nullptr);
    if (!button) {
        return;
    }
    button->setAvailable(available);
    if (!available && button->isSelected() && m_selectedXrayType == type) {
        m_selectedXrayType = -1;
    }
}

void QualityControlBaseView::setXrayTypeSelected(int type)
{
    if (m_isSafeMode) {
        return;
    }
    XrayImageButton *button = m_xrayButtons.value(type, nullptr);
    if (!button || !button->isAvailable()) {
        return;
    }
    handleXrayButtonClicked(button);
}

void QualityControlBaseView::clearXraySelection()
{
    for (XrayImageButton *button : m_tongfangButtons) {
        button->setSelected(false);
    }
    for (XrayImageButton *button : m_yisuoButtons) {
        button->setSelected(false);
    }
    m_selectedXrayType = -1;
}

QList<AnnotationGraphicsView::AnnotationData> QualityControlBaseView::mainAnnotations() const
{
    if (!m_mainView) {
        return {};
    }
    return m_mainView->annotations();
}

QList<AnnotationGraphicsView::AnnotationData> QualityControlBaseView::auxAnnotations() const
{
    if (!m_auxView) {
        return {};
    }
    return m_auxView->annotations();
}

void QualityControlBaseView::setJudgeResultText(const QString &text)
{
    if (m_judgeValueLabel) {
        m_judgeValueLabel->setText(text);
    }
}

void QualityControlBaseView::setFreshnessText(const QString &text)
{
    if (m_freshnessValueLabel) {
        m_freshnessValueLabel->setText(text);
    }
}

void QualityControlBaseView::setChannelText(const QString &text)
{
    if (m_channelValueLabel) {
        m_channelValueLabel->setText(text);
    }
}

void QualityControlBaseView::setTimeText(const QString &text)
{
    if (m_timeValueLabel) {
        m_timeValueLabel->setText(text);
    }
}

void QualityControlBaseView::clearBaseViewState()
{
    clearTaskImageState();
    clearXraySelection();
    setAvailableXrayTypes(QSet<int>());

    if (m_mainView) {
        m_mainView->setImage(QPixmap());
        m_mainView->clearAnnotations();
        m_mainView->resetZoom();
    }
    if (m_auxView) {
        m_auxView->setImage(QPixmap());
        m_auxView->clearAnnotations();
        m_auxView->resetZoom();
    }
    if (m_thumbnail) {
        m_thumbnail->setImage(QPixmap());
        m_thumbnail->setViewRectNormalized(QRectF());
    }
    if (m_layerView) {
        m_layerView->setXrayItems(QList<XrayImage>());
        m_layerView->setDurationText(QStringLiteral("--"));
        m_layerView->setTopImage(QPixmap());
    }
    if (m_activeView) {
        m_activeView->setActive(false);
    }
    m_activeView = m_mainView;
    if (m_activeView) {
        m_activeView->setActive(true);
    }

    setJudgeResultText(QStringLiteral("--"));
    setFreshnessText(QStringLiteral("--"));
    setChannelText(QStringLiteral("--"));
    setTimeText(QStringLiteral("--"));
    setBottomBarTaskActive(false);
}

void QualityControlBaseView::setBottomBarTaskActive(bool hasTask)
{
    m_bottomBarTaskActive = hasTask;
    const bool effectiveHasTask = hasTask && !m_isSafeMode;
    auto applyButtonState = [](QAbstractButton *button, bool enabled) {
        if (!button) {
            return;
        }
        button->setEnabled(enabled);
        button->setCursor(enabled ? Qt::PointingHandCursor : Qt::ArrowCursor);
    };

    if (m_brandStack) {
        m_brandStack->setEnabled(effectiveHasTask && m_currentBrandType != ServerType::Unknown);
    }
    for (auto it = m_xrayButtons.begin(); it != m_xrayButtons.end(); ++it) {
        XrayImageButton *button = it.value();
        if (button) {
            button->setEnabled(!m_isSafeMode);
            button->setCursor(effectiveHasTask ? Qt::PointingHandCursor : Qt::ArrowCursor);
        }
    }
    applyButtonState(m_escButton, effectiveHasTask);
    applyButtonState(m_zoomInButton, effectiveHasTask);
    applyButtonState(m_zoomOutButton, effectiveHasTask);
    // 安全态下仍允许“开检/放行”，但等待原图门禁阶段需要临时禁用。
    const bool submitEnabled = hasTask && !m_originSubmitBlocked;
    applyButtonState(m_startCheckButton, submitEnabled);
    applyButtonState(m_passButton, submitEnabled);
    applyButtonState(m_detailButton, effectiveHasTask);
    applyButtonState(m_aiToggleButton, effectiveHasTask);
    qCInfo(lcQcView) << "[BaseView] bottom bar state hasTask=" << hasTask
                     << "safeMode=" << m_isSafeMode
                     << "originSubmitBlocked=" << m_originSubmitBlocked
                     << "effectiveHasTask=" << effectiveHasTask
                     << "brandStackEnabled=" << (m_brandStack ? m_brandStack->isEnabled() : false);
}

void QualityControlBaseView::handleViewActivated(AnnotationGraphicsView *view)
{
    if (m_activeView == view) {
        return;
    }
    if (m_activeView) {
        m_activeView->setActive(false);
    }
    m_activeView = view;
    if (m_activeView) {
        m_activeView->setActive(true);
    }
    if (m_thumbnail && m_activeView && m_activeView->hasImage()) {
        m_thumbnail->setImage(m_activeView->image());
        m_activeView->refreshViewRect();
    }
}

bool QualityControlBaseView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_middleArea && event->type() == QEvent::Resize) {
        updateOverlayGeometry();
    }
    return QWidget::eventFilter(watched, event);
}

void QualityControlBaseView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    setShortcutsEnabled(true);
}

void QualityControlBaseView::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    setShortcutsEnabled(false);
}

void QualityControlBaseView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate) {
        setShortcutsEnabled(true);
    } else if (event->type() == QEvent::WindowDeactivate) {
        setShortcutsEnabled(false);
    }
    QWidget::changeEvent(event);
}

QWidget *QualityControlBaseView::buildMiddleArea()
{
    QWidget *middle = new QWidget(contentParent());
    m_middleArea = middle;
    middle->setMinimumHeight(898);
    middle->setMaximumHeight(898);
    middle->setStyleSheet(QStringLiteral("background:#F2F2F2;"));
    middle->installEventFilter(this);

    QHBoxLayout *layout = new QHBoxLayout(middle);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    m_mainView = new AnnotationGraphicsView(middle);
    m_auxView = new AnnotationGraphicsView(middle);

    connect(m_mainView, &AnnotationGraphicsView::activated, this, &QualityControlBaseView::handleViewActivated);
    connect(m_auxView, &AnnotationGraphicsView::activated, this, &QualityControlBaseView::handleViewActivated);

    layout->addWidget(m_mainView);
    layout->addWidget(m_auxView);

    handleViewActivated(m_mainView);

    m_overlayPanel = new CollapsiblePanel(middle);
    m_overlayPanel->setStyleSheet(QStringLiteral(
        "QWidget#collapsiblePanel{background:#d7d7d7;border:1px solid #bdbdbd;border-radius:4px;}"
        "QPushButton#collapsibleToggle{background:#1e8caa;color:white;border:none;}"
        "QPushButton#collapsibleToggle:hover{background:#2b9bbc;}"
    ));

    m_layerView = new LayerViewWidget(m_overlayPanel);
    m_overlayPanel->setContentWidget(m_layerView);

    connect(m_layerView, &LayerViewWidget::selectionChanged, this, [this](int index) {
        applyOverlaySelection(index, true);
    });

    connect(m_overlayPanel, &CollapsiblePanel::collapsedChanged, this, [this]() {
        updateOverlayGeometry();
    });
    connect(m_layerView, &LayerViewWidget::pageChanged, this, [this](int startIndex) {
        if (m_waitingOriginReady) {
            qCDebug(lcQcView) << "[BaseView] skip lazy preload while waiting origin";
            return;
        }
        QStringList lazyUrls;
        const int endIndex = qMin(startIndex + m_layerView->pageSize(), m_overlayItems.size());
        for (int i = startIndex; i < endIndex; ++i) {
            appendUnique(lazyUrls, collectImageUrls(m_overlayItems.at(i)));
        }
        preloadTaskImageUrls(lazyUrls);
    });
    updateOverlayGeometry();
    return middle;
}

QWidget *QualityControlBaseView::buildBottomBar()
{
    QWidget *bottom = new QWidget(contentParent());
    bottom->setStyleSheet(QStringLiteral("background-color: #ffffff;"));

    bottom->setMinimumHeight(127);
    bottom->setMaximumHeight(127);

    QHBoxLayout *layout = new QHBoxLayout(bottom);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(15);


    QWidget *thumbnailAndToolsPanel = new QWidget(bottom);
    thumbnailAndToolsPanel->setStyleSheet(QStringLiteral("background-color: #e5e5e5;"));
    QHBoxLayout *thumbnailAndToolsLayout = new QHBoxLayout(thumbnailAndToolsPanel);
    thumbnailAndToolsLayout->setContentsMargins(0, 0, 0, 0);
    thumbnailAndToolsLayout->setSpacing(0); // 保持原有间距

    m_thumbnail = new ThumbnailWidget(thumbnailAndToolsPanel);
    m_thumbnail->setFixedSize(150, 100);
    m_thumbnail->setStyleSheet(QStringLiteral("border:1px solid #c0c0c0;"));

    QWidget *tools = new QWidget(thumbnailAndToolsPanel);
    tools->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QHBoxLayout *toolsLayout = new QHBoxLayout(tools);
    toolsLayout->setContentsMargins(0, 0, 0, 0);
    toolsLayout->setSpacing(6);

    m_brandStack = new QStackedWidget(tools);
    m_brandStack->addWidget(buildBrandButtons(ServerType::TongFang, tongfangSpecs(), &m_tongfangButtons));
    m_brandStack->addWidget(buildBrandButtons(ServerType::YiSuo, yisuoSpecs(), &m_yisuoButtons));
    m_brandStack->setCurrentIndex(0);
    selectToolButtonsByBrand(m_currentBrandType);

    toolsLayout->addWidget(m_brandStack);

    thumbnailAndToolsLayout->addWidget(m_thumbnail);
    thumbnailAndToolsLayout->addWidget(tools);

    // ---------------- 第二部分：操作按钮区 ----------------
    QWidget *operationPanel = new QWidget(bottom);
    operationPanel->setStyleSheet(QStringLiteral("background-color: #e5e5e5;"));
    QVBoxLayout *actionLayout = new QVBoxLayout(operationPanel);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(6);
    actionLayout->setAlignment(Qt::AlignCenter); // 按钮居中

    auto makeActionButton = [operationPanel](const QString &text, const QString &style) {
        QPushButton *button = new QPushButton(text, operationPanel);
        button->setMinimumSize(170, 42);
        button->setStyleSheet(
            style +
            QStringLiteral("QPushButton:disabled{background:#b8b8b8;color:#f0f0f0;}")
        );
        return button;
    };

    QWidget *actionRow1 = new QWidget(operationPanel);
    QHBoxLayout *actionRow1Layout = new QHBoxLayout(actionRow1);
    actionRow1Layout->setContentsMargins(0, 0, 0, 10);
    actionRow1Layout->setSpacing(30);
    m_startCheckButton = makeActionButton(QStringLiteral("开        检"),
                                          QStringLiteral("QPushButton{background:#c53a2b;color:white;border-radius:6px;padding:6px 16px;font-size:11pt;font-weight:bold;}"));
    m_passButton = makeActionButton(QStringLiteral("放        行"),
                                    QStringLiteral("QPushButton{background:#2dbb63;color:white;border-radius:6px;padding:6px 16px;font-size:11pt;font-weight:bold;}"));
    actionRow1Layout->addWidget(m_startCheckButton);
    actionRow1Layout->addWidget(m_passButton);

    QWidget *actionRow2 = new QWidget(operationPanel);
    QHBoxLayout *actionRow2Layout = new QHBoxLayout(actionRow2);
    actionRow2Layout->setContentsMargins(0, 0, 0, 0);
    actionRow2Layout->setSpacing(30);
    m_detailButton = makeActionButton(QStringLiteral("人包详情"),
                                      QStringLiteral("QPushButton{background:#27b4e6;color:white;border-radius:6px;padding:6px 16px;font-size:11pt;font-weight:bold;}"));
    m_aiToggleButton = makeActionButton(QStringLiteral("隐藏/显示AI"),
                                        QStringLiteral("QPushButton{background:#80b7d8;color:white;border-radius:6px;padding:6px 16px;font-size:11pt;font-weight:bold;}"));
    actionRow2Layout->addWidget(m_detailButton);
    actionRow2Layout->addWidget(m_aiToggleButton);
    connect(m_startCheckButton, &QPushButton::clicked, this, &QualityControlBaseView::startCheckRequested);
    connect(m_passButton, &QPushButton::clicked, this, &QualityControlBaseView::passRequested);
    connect(m_detailButton, &QPushButton::clicked, this, &QualityControlBaseView::detailRequested);
    actionLayout->addWidget(actionRow1);
    actionLayout->addWidget(actionRow2);

    // ---------------- 第三部分：信息面板 + 徽章区 ----------------
    QWidget *infoAndBadgePanel = new QWidget(bottom);
    infoAndBadgePanel->setStyleSheet(QStringLiteral("background-color: #e5e5e5;"));
    QHBoxLayout *rightLayout = new QHBoxLayout(infoAndBadgePanel);
    rightLayout->setContentsMargins(0, 0, 20, 0);
    rightLayout->setSpacing(40); // 信息区和徽章之间间距加大
    rightLayout->setAlignment(Qt::AlignVCenter);

    QWidget *infoPanel = new QWidget(infoAndBadgePanel);
    QGridLayout *infoLayout = new QGridLayout(infoPanel);
    infoLayout->setContentsMargins(70, 0, 0, 0);
    infoLayout->setHorizontalSpacing(10); // 信息项之间水平间距加大
    infoLayout->setVerticalSpacing(25); // 信息项之间垂直间距加大
    infoLayout->setAlignment(Qt::AlignVCenter);
    auto makeInfoValue = [infoPanel](const QString &text) {
        QLabel *label = new QLabel(text, infoPanel);
        // 修复核心：QStringLiteral拼接必须加 + 号
        label->setStyleSheet(
            QStringLiteral("QLabel{background:#f1f1f1; padding:0px 10px 0px 5px;border-radius:6px; font-size:16px;}") +
            QStringLiteral("color:%1;").arg(text == QStringLiteral("放行") ? "#2dbb63" : "#333333")
            );
        label->setMinimumWidth(170); // 信息框最小宽度加大，更舒展
        label->setMinimumHeight(35); // 信息框最小宽度加大，更舒展
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        return label;
    };

    QLabel *judgeTitle = new QLabel(QStringLiteral("判图结论："), infoPanel);
    judgeTitle->setStyleSheet(QStringLiteral("font-size:18px;font-weight:bold;")); // 标题字体放大到20px
    judgeTitle->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_judgeValueLabel = makeInfoValue(QStringLiteral("放行"));

    QLabel *timeTitle = new QLabel(QStringLiteral("时间："), infoPanel);
    timeTitle->setStyleSheet(QStringLiteral("font-size:18px;font-weight:bold;"));
    timeTitle->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_timeValueLabel = makeInfoValue(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));

    QLabel *sharpTitle = new QLabel(QStringLiteral("图像鲜度："), infoPanel); // 修正为“图像鲜度”匹配截图
    sharpTitle->setStyleSheet(QStringLiteral("font-size:18px;font-weight:bold;"));
    sharpTitle->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_freshnessValueLabel = makeInfoValue(QStringLiteral("--"));

    QLabel *channelTitle = new QLabel(QStringLiteral("通道："), infoPanel);
    channelTitle->setStyleSheet(QStringLiteral("font-size:18px;font-weight:bold;"));
    channelTitle->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_channelValueLabel = makeInfoValue(QStringLiteral("LJ09")); // 通道值改为LJ09匹配截图

    // 第一行：判图结论+放行（紧凑） + 组间弹簧(30px) + 时间+时间值（紧凑）
    infoLayout->addWidget(judgeTitle, 0, 0);
    infoLayout->addWidget(m_judgeValueLabel, 0, 1);
    // 仅在放行值和时间标题之间加30px固定弹簧（组间间距）
    infoLayout->addItem(new QSpacerItem(25, 1, QSizePolicy::Fixed, QSizePolicy::Minimum), 0, 2);
    infoLayout->addWidget(timeTitle, 0, 3);
    infoLayout->addWidget(m_timeValueLabel, 0, 4);

    // 第二行：图像鲜度+--（紧凑） + 组间弹簧(30px) + 通道+LJ09（紧凑）
    infoLayout->addWidget(sharpTitle, 1, 0);
    infoLayout->addWidget(m_freshnessValueLabel, 1, 1);
    // 仅在--值和通道标题之间加30px固定弹簧（组间间距）
    infoLayout->addItem(new QSpacerItem(25, 1, QSizePolicy::Fixed, QSizePolicy::Minimum), 1, 2);
    infoLayout->addWidget(channelTitle, 1, 3);
    infoLayout->addWidget(m_channelValueLabel, 1, 4);

    QWidget *badge = new QWidget(infoAndBadgePanel);
    badge->setFixedWidth(210); // 固定宽度210px，和要求一致
    badge->setMinimumHeight(80); // 匹配截图高度比例
    badge->setStyleSheet(QStringLiteral("background-color: #ffffff; border-radius:6px;"));
    QHBoxLayout *badgeLayout = new QHBoxLayout(badge);
    badgeLayout->setContentsMargins(0, 0, 0, 0);
    badgeLayout->setSpacing(10);
    badgeLayout->setAlignment(Qt::AlignCenter);

    QLabel *badgeIcon = new QLabel(badge);
    badgeIcon->setFixedSize(60, 60); // 徽章图标稍大
    badgeIcon->setAlignment(Qt::AlignCenter);
    badgeIcon->setStyleSheet(QStringLiteral("QLabel{background:#e9f0ff;border-radius:24px;}"));
    badgeIcon->setPixmap(QPixmap(QStringLiteral(":/Images/robot.png")).scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel *badgeText = new QLabel(QStringLiteral("首检图像"), badge);
    badgeText->setStyleSheet(QStringLiteral("QLabel{font-weight:bold;font-size:22px;}")); // 徽章文字放大到18px
    badgeLayout->addWidget(badgeIcon);
    badgeLayout->addWidget(badgeText);

    rightLayout->addWidget(infoPanel);
    rightLayout->addItem(new QSpacerItem(20, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
    rightLayout->addWidget(badge);


    // ---------------- 组装主布局并设置比例 ----------------
    layout->addWidget(thumbnailAndToolsPanel);  // 第一部分：缩略图+工具
    layout->addWidget(operationPanel);          // 第二部分：操作按钮
    layout->addWidget(infoAndBadgePanel);       // 第三部分：信息+徽章
    // 核心：设置拉伸权重（1:1:2，对应原475:460:955≈1:0.97:2.01）
    layout->setStretch(0, 110);  // thumbnailAndToolsPanel 权重1
    layout->setStretch(1, 97);  // operationPanel 权重1
    layout->setStretch(2, 190);  // infoAndBadgePanel 权重2



    connect(m_thumbnail, &ThumbnailWidget::clickedAtNormalized, this, [this](qreal xRatio, qreal yRatio) {
        if (m_activeView && m_activeView->isZoomed()) {
            m_activeView->centerOnNormalized(xRatio, yRatio);
        }
    });

    auto bindView = [this](AnnotationGraphicsView *view) {
        connect(view, &AnnotationGraphicsView::viewRectChanged, this, [this, view](const QRectF &rect) {
            if (m_thumbnail && m_activeView == view) {
                m_thumbnail->setViewRectNormalized(rect);
            }
        });
    };
    if (m_mainView) {
        bindView(m_mainView);
    }
    if (m_auxView) {
        bindView(m_auxView);
    }

    if (!m_shortcutManager) {
        m_shortcutManager = new QualityControlShortcutManager(this, this);
        QHash<int, QAbstractButton*> typeButtons;
        for (auto it = m_xrayButtons.begin(); it != m_xrayButtons.end(); ++it) {
            typeButtons.insert(it.key(), it.value());
        }
        m_shortcutManager->setServerType(m_currentBrandType);
        m_shortcutManager->configure(typeButtons, m_escButton, m_startCheckButton, m_passButton, m_detailButton);
    }
    updateToolIconsByBrand();

    return bottom;
}

QWidget *QualityControlBaseView::buildBrandButtons(ServerType brandType, const QList<XrayButtonSpec> &specs, QList<XrayImageButton*> *buttonList)
{
    QWidget *container = new QWidget(contentParent());
    QGridLayout *layout = new QGridLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setHorizontalSpacing(14);
    layout->setVerticalSpacing(10);

    const int columns = 5;
    int index = 0;
    for (const XrayButtonSpec &spec : specs) {
        XrayImageButton *btn = new XrayImageButton(spec.typeId, spec.label, container);
        btn->setImages(spec.unavailableImage, spec.availableImage, spec.selectedImage);
        btn->setAvailable(false);
        layout->addWidget(btn, index / columns, index % columns);
        m_xrayButtons.insert(spec.typeId, btn);
        if (buttonList) {
            buttonList->append(btn);
        }
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            handleXrayButtonClicked(btn);
        });
        ++index;
    }

    auto makeIconButton = [container](const QString &iconPath, const QString &tooltip) {
        QPushButton *button = new QPushButton(container);
        button->setFixedSize(47, 47);
        button->setIcon(QIcon(iconPath));
        button->setIconSize(QSize(47, 47));
        button->setFlat(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setToolTip(tooltip);
        button->setStyleSheet(QStringLiteral("QPushButton{border:none;background:transparent;}"));
        return button;
    };

    const ToolIconSet iconSet = toolIconSetForServerType(brandType);
    QPushButton *escButton = makeIconButton(QString::fromLatin1(iconSet.esc), QStringLiteral("ESC"));
    QPushButton *zoomInButton = makeIconButton(QString::fromLatin1(iconSet.zoomIn), QStringLiteral("放大"));
    QPushButton *zoomOutButton = makeIconButton(QString::fromLatin1(iconSet.zoomOut), QStringLiteral("缩小"));

    connect(escButton, &QPushButton::clicked, this, [this]() {
        qCDebug(lcQcView) << "[BaseView] ESC clicked";

        // ESC 语义：回到原图（右上角 Overlay 同步选中原图）+ 恢复缩放比例。
        int originIndex = -1;
        for (int i = 0; i < m_overlayItems.size(); ++i) {
            const int overlayType = imageTypeOf(m_overlayItems.at(i));
            if (overlayType == XImage_Origin || overlayType == XImage_HM_Origin
                || overlayType == XImage_YS_Origin || overlayType == XImage_TF_Origin) {
                originIndex = i;
                break;
            }
        }
        if (originIndex < 0 && !m_overlayItems.isEmpty()) {
            originIndex = 0;
        }
        if (originIndex >= 0) {
            if (m_layerView) {
                m_layerView->setSelectedIndex(originIndex);
            } else {
                applyOverlaySelection(originIndex, true);
            }
            qCInfo(lcQcView) << "[BaseView] ESC switch to origin index" << originIndex;
        }

        if (m_mainView) {
            m_mainView->resetZoom();
        }
        if (m_auxView) {
            m_auxView->resetZoom();
        }
    });
    connect(zoomInButton, &QPushButton::clicked, this, [this]() {
        qCDebug(lcQcView) << "[BaseView] ZoomIn clicked activeView=" << (m_activeView != nullptr);
        if (m_activeView) {
            m_activeView->zoomIn();
        }
    });
    connect(zoomOutButton, &QPushButton::clicked, this, [this]() {
        qCDebug(lcQcView) << "[BaseView] ZoomOut clicked activeView=" << (m_activeView != nullptr);
        if (m_activeView) {
            m_activeView->zoomOut();
        }
    });


    layout->addWidget(escButton, 1, 1);
    layout->addWidget(zoomInButton, 1, 2);
    layout->addWidget(zoomOutButton, 1, 3);

    const int brandKey = static_cast<int>(brandType);
    m_escButtonsByBrand.insert(brandKey, escButton);
    m_zoomInButtonsByBrand.insert(brandKey, zoomInButton);
    m_zoomOutButtonsByBrand.insert(brandKey, zoomOutButton);
    return container;
}

QWidget *QualityControlBaseView::contentParent() const
{
    return m_layoutHost ? m_layoutHost : const_cast<QualityControlBaseView*>(this);
}

void QualityControlBaseView::handleXrayButtonClicked(XrayImageButton *button)
{
    if (m_isSafeMode) {
        return;
    }
    if (!button) {
        return;
    }
    if (!button->isAvailable()) {
        qCInfo(lcQcView) << "[BaseView] xray button unavailable clicked type=" << button->typeId();
        emit xrayTypeUnavailableClicked(button->typeId());
        return;
    }
    qCInfo(lcQcView) << "[BaseView] xray button clicked type=" << button->typeId()
                     << "selected=" << button->isSelected();

    const int type = button->typeId();
    // 非 pb 可联动类型统一处理：不改变按钮选中态，不切主/辅图，仅通知业务层。
    if (!m_intersectionXrayTypes.contains(type)) {
        qCDebug(lcQcView) << "[BaseView] type" << type << "not linkable by pb, skip image switching";
        emit xrayTypeToggled(type, true);
        return;
    }

    if (button->isSelected()) {
        // 二次点击同一可联动按钮：回到原图，并同步右上角 Overlay 选中到原图。
        int originIndex = -1;
        for (int i = 0; i < m_overlayItems.size(); ++i) {
            const int overlayType = imageTypeOf(m_overlayItems.at(i));
            if (overlayType == XImage_Origin || overlayType == XImage_HM_Origin
                || overlayType == XImage_YS_Origin || overlayType == XImage_TF_Origin) {
                originIndex = i;
                break;
            }
        }
        if (originIndex < 0 && !m_overlayItems.isEmpty()) {
            originIndex = 0;
        }
        if (originIndex >= 0) {
            if (m_layerView) {
                m_layerView->setSelectedIndex(originIndex);
            } else {
                applyOverlaySelection(originIndex, true);
            }
            qCInfo(lcQcView) << "[BaseView] type" << type << "re-clicked, switch back to origin index" << originIndex;
        } else {
            clearXraySelection();
            qCWarning(lcQcView) << "[BaseView] type" << type << "re-clicked, but no origin item found";
        }
        emit xrayTypeToggled(type, false);
        return;
    }
    if (!selectTypeFromOverlayOrFallback(type, true, true, true)) {
        // 非交集类型：不改变按钮选中态，仅通知业务层切换请求。
        emit xrayTypeToggled(type, true);
    }
}

void QualityControlBaseView::setShortcutsEnabled(bool enabled)
{
    m_shortcutsEnabled = enabled;
    if (m_shortcutManager) {
        m_shortcutManager->setEnabled(enabled);
    }
}

void QualityControlBaseView::applySafeMode(bool enabled)
{
    if (m_isSafeMode == enabled) {
        return;
    }

    m_isSafeMode = enabled;
    if (enabled) {
        clearTaskImageState();
        clearXraySelection();
        setAvailableXrayTypes(QSet<int>());
        setMainImage(QPixmap());
        setAuxImage(QPixmap());
        if (m_thumbnail) {
            m_thumbnail->setViewRectNormalized(QRectF());
            m_thumbnail->setEnabled(false);
        }
        if (m_layerView) {
            m_layerView->setXrayItems(QList<XrayImage>());
            m_layerView->setDurationText(QStringLiteral("--"));
            m_layerView->setTopImage(QPixmap());
            m_layerView->setEnabled(false);
        }
        if (m_overlayPanel) {
            m_overlayPanel->setEnabled(false);
        }
        if (m_mainView) {
            m_mainView->setEnabled(false);
        }
        if (m_auxView) {
            m_auxView->setEnabled(false);
        }
        if (m_shortcutManager) {
            m_shortcutManager->setServerType(ServerType::Unknown);
        }
        qCWarning(lcQcView) << "[BaseView] safe mode enabled";
    } else {
        if (m_thumbnail) {
            m_thumbnail->setEnabled(true);
        }
        if (m_layerView) {
            m_layerView->setEnabled(true);
        }
        if (m_overlayPanel) {
            m_overlayPanel->setEnabled(true);
        }
        if (m_mainView) {
            m_mainView->setEnabled(true);
        }
        if (m_auxView) {
            m_auxView->setEnabled(true);
        }
        if (m_shortcutManager) {
            m_shortcutManager->setServerType(m_currentBrandType);
        }
        qCInfo(lcQcView) << "[BaseView] safe mode disabled, serverType=" << static_cast<int>(m_currentBrandType);
    }

    if (m_shortcutManager) {
        m_shortcutManager->setEnabled(m_shortcutsEnabled);
    }
    setBottomBarTaskActive(m_bottomBarTaskActive);
}

void QualityControlBaseView::positionOverlayPanel()
{
    if (!m_middleArea || !m_overlayPanel) {
        return;
    }
    m_overlayPanel->adjustSize();
    const int margin = 0;
    int x = m_middleArea->width() - m_overlayPanel->width() - margin;
    int y = margin;
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    m_overlayPanel->move(x, y);
    m_overlayPanel->raise();
}

void QualityControlBaseView::updateOverlayGeometry()
{
    positionOverlayPanel();
}

void QualityControlBaseView::selectToolButtonsByBrand(ServerType brandType)
{
    const int requestedKey = static_cast<int>(brandType);
    const int fallbackKey = static_cast<int>(ServerType::TongFang);
    m_escButton = m_escButtonsByBrand.value(requestedKey, m_escButtonsByBrand.value(fallbackKey, nullptr));
    m_zoomInButton = m_zoomInButtonsByBrand.value(requestedKey, m_zoomInButtonsByBrand.value(fallbackKey, nullptr));
    m_zoomOutButton = m_zoomOutButtonsByBrand.value(requestedKey, m_zoomOutButtonsByBrand.value(fallbackKey, nullptr));
}

void QualityControlBaseView::updateToolIconsByBrand()
{
    if (!m_escButton || !m_zoomInButton || !m_zoomOutButton) {
        return;
    }

    const ToolIconSet iconSet = toolIconSetForServerType(m_currentBrandType);
    m_escButton->setIcon(QIcon(QString::fromLatin1(iconSet.esc)));
    m_zoomInButton->setIcon(QIcon(QString::fromLatin1(iconSet.zoomIn)));
    m_zoomOutButton->setIcon(QIcon(QString::fromLatin1(iconSet.zoomOut)));
    qCInfo(lcQcView) << "[BaseView] tool icons switched by brand="
                     << iconSet.brandName
                     << "serverType=" << static_cast<int>(m_currentBrandType);
}

void QualityControlBaseView::configureTaskImages(const ImageDistributeInfo &info)
{
    if (m_isSafeMode) {
        qCWarning(lcQcView) << "[BaseView] configureTaskImages ignored in safe mode, taskId=" << info.id;
        return;
    }
    if (m_layerView) {
        m_layerView->setDurationText(overlayDurationText(info.viewDuration));
    }
    qCInfo(lcQcView) << "[BaseView] configureTaskImages taskId=" << info.id
                     << "pbCount=" << info.pbXrayImages.size()
                     << "xrayCount=" << info.xrayImages.size();
    // 每次切任务先清空任务级状态，避免串图/串选中态。
    clearTaskImageState();
    // Overlay 图像来源严格使用 pbXrayImages。
    m_overlayItems = buildOverlayItems(info);
    if (m_overlayItems.isEmpty()) {
        qCWarning(lcQcView) << "[BaseView] overlay is empty, clear main/aux image";
        clearXraySelection();
        setAvailableXrayTypes(QSet<int>());
        rebuildOverlayLayerCards();
        setMainImage(QPixmap());
        setAuxImage(QPixmap());
        return;
    }

    // 可用态按并集；点击后的图像联动按 pbXrayImages 可映射到按钮的类型。
    const QSet<int> pbTypes = imageTypesOf(info.pbXrayImages);
    const QSet<int> xrayTypes = imageTypesOf(info.xrayImages);
    m_availableXrayTypes = pbTypes;
    m_availableXrayTypes.unite(xrayTypes);
    m_intersectionXrayTypes.clear();
    setAvailableXrayTypes(m_availableXrayTypes);

    m_overlayIndexByType.clear();
    for (int i = 0; i < m_overlayItems.size(); ++i) {
        const int type = imageTypeOf(m_overlayItems.at(i));
        if (type >= 0 && !m_overlayIndexByType.contains(type)) {
            m_overlayIndexByType.insert(type, i);
        }
    }
    for (auto it = m_overlayIndexByType.cbegin(); it != m_overlayIndexByType.cend(); ++it) {
        if (m_xrayButtons.contains(it.key())) {
            m_intersectionXrayTypes.insert(it.key());
        }
    }
    qCInfo(lcQcView) << "[BaseView] type sets"
                     << "pb=" << intSetToText(pbTypes)
                     << "xray=" << intSetToText(xrayTypes)
                     << "available(union)=" << intSetToText(m_availableXrayTypes)
                     << "linkable(pb)=" << intSetToText(m_intersectionXrayTypes);
    m_fallbackImageByType.clear();
    for (const XrayImage &img : info.xrayImages) {
        const int type = imageTypeOf(img);
        if (type >= 0 && !m_fallbackImageByType.contains(type)) {
            m_fallbackImageByType.insert(type, img);
        }
    }

    // 首屏默认选中“原图”类型；找不到原图则用第一张。
    int initialIndex = 0;
    for (int i = 0; i < m_overlayItems.size(); ++i) {
        const int type = imageTypeOf(m_overlayItems.at(i));
        if (type == XImage_Origin || type == XImage_HM_Origin || type == XImage_YS_Origin || type == XImage_TF_Origin) {
            initialIndex = i;
            break;
        }
    }
    for (int i = 0; i < m_overlayItems.size(); ++i) {
        m_overlayItems[i].setSelected(i == initialIndex);
    }

    m_taskOriginMainUrl = originMainUrl();
    m_taskOriginAssistUrl = originAssistUrl();

    rebuildOverlayLayerCards();

    // 预取优先级：当前选中项 + Overlay 首屏三张 + 交集类型对应图片。
    QStringList priorityUrls;
    appendUnique(priorityUrls, collectImageUrls(m_overlayItems.value(initialIndex)));
    const int firstPageCount = qMin(3, m_overlayItems.size());
    for (int i = 0; i < firstPageCount; ++i) {
        appendUnique(priorityUrls, collectImageUrls(m_overlayItems.at(i)));
    }
    for (int type : m_availableXrayTypes) {
        if (m_overlayIndexByType.contains(type)) {
            appendUnique(priorityUrls, collectImageUrls(m_overlayItems.at(m_overlayIndexByType.value(type))));
        }
        if (m_fallbackImageByType.contains(type)) {
            appendUnique(priorityUrls, collectImageUrls(m_fallbackImageByType.value(type)));
        }
    }
    const bool needWaitOrigin = !isOriginPairReady();
    if (needWaitOrigin) {
        m_taskDeferredUrls.clear();
        for (const QString &url : priorityUrls) {
            if (url.isEmpty()) {
                continue;
            }
            if (url == m_taskOriginMainUrl || url == m_taskOriginAssistUrl) {
                continue;
            }
            m_taskDeferredUrls.insert(url);
        }
        startOriginGate(initialIndex);
        preloadTaskImageUrls(QStringList{m_taskOriginMainUrl, m_taskOriginAssistUrl});
        qCInfo(lcQcView) << "[BaseView] preload priority urls=" << priorityUrls.size()
                         << "initialIndex=" << initialIndex
                         << "deferred=" << m_taskDeferredUrls.size();
        qCInfo(lcQcView) << "[BaseView] waiting origin pair"
                         << "main=" << m_taskOriginMainUrl
                         << "assist=" << m_taskOriginAssistUrl;
        setMainImage(QPixmap());
        setAuxImage(QPixmap());
        return;
    }

    preloadTaskImageUrls(priorityUrls);
    qCInfo(lcQcView) << "[BaseView] preload priority urls=" << priorityUrls.size()
                     << "initialIndex=" << initialIndex;
    m_originSubmitBlocked = false;
    m_waitingOriginReady = false;
    if (m_originGateTimer) {
        m_originGateTimer->stop();
    }
    setBottomBarTaskActive(m_bottomBarTaskActive);
    applyOverlaySelection(initialIndex, true);
}

void QualityControlBaseView::clearTaskImageState()
{
    // 仅清理“当前任务”缓存与索引，不影响全局 ImageProvider。
    m_overlayItems.clear();
    m_overlayIndexByType.clear();
    m_fallbackImageByType.clear();
    m_availableXrayTypes.clear();
    m_intersectionXrayTypes.clear();
    m_taskImageCache.clear();
    m_taskPendingUrls.clear();
    m_taskDeferredUrls.clear();
    m_taskOriginMainUrl.clear();
    m_taskOriginAssistUrl.clear();
    m_waitingOriginReady = false;
    m_originSubmitBlocked = false;
    m_currentMainUrl.clear();
    m_currentAuxUrl.clear();
    m_pendingType = -1;
    m_pendingOverlayIndex = -1;
    m_pendingMainUrl.clear();
    m_pendingAuxUrl.clear();
    m_originInitialIndex = 0;
    if (m_originGateTimer) {
        m_originGateTimer->stop();
    }
}

bool QualityControlBaseView::selectTypeFromOverlayOrFallback(int type, bool syncOverlay, bool syncButtons, bool emitToggle)
{
    // 按钮可用态是并集，但点击后的图像联动仅对 pb 可映射类型生效。
    if (!m_intersectionXrayTypes.contains(type)) {
        qCDebug(lcQcView) << "[BaseView] type" << type << "not linkable by pb, skip image switching";
        return false;
    }

    // 优先走 overlay 映射（overlay 来源就是 pbXrayImages），保证按钮点击优先取 pb 图。
    if (m_overlayIndexByType.contains(type)) {
        const int index = m_overlayIndexByType.value(type);
        if (syncOverlay && m_layerView) {
            m_layerView->setSelectedIndex(index);
        }
        applyOverlaySelection(index, syncButtons);
        if (emitToggle) {
            emit xrayTypeToggled(type, true);
        }
        qCDebug(lcQcView) << "[BaseView] type" << type << "handled by overlay index" << index;
        return true;
    }

    if (!m_fallbackImageByType.contains(type)) {
        return false;
    }

    // 若 overlay 不含该类型，则回退到 xrayImages 的同类型图。
    const XrayImage fallback = m_fallbackImageByType.value(type);
    preloadTaskImageUrls(collectImageUrls(fallback));
    const QString mainUrl = fallback.mainXrayImageUrl();
    const QString assistUrl = fallback.assistXrayImageUrl();
    m_currentMainUrl = mainUrl;
    m_currentAuxUrl = assistUrl;
    const QPixmap mainPix = resolveTaskPixmap(mainUrl);
    const QPixmap auxPix = resolveTaskPixmap(assistUrl);
    const bool targetReady = isDisplayTargetReady(mainUrl, assistUrl);
    if (m_waitingOriginReady || !targetReady) {
        queuePendingDisplayTarget(type, -1, mainUrl, assistUrl);
        if ((!m_mainView || !m_mainView->hasImage()) && !mainPix.isNull()) {
            setMainImage(mainPix);
        }
        if ((!m_auxView || !m_auxView->hasImage()) && !auxPix.isNull()) {
            setAuxImage(auxPix);
        }
        qCDebug(lcQcView) << "[BaseView] fallback target pending type=" << type
                          << "originGate=" << m_waitingOriginReady
                          << "mainReady=" << isOriginImageReady(mainUrl)
                          << "assistReady=" << isOriginImageReady(assistUrl);
    } else {
        if (m_pendingMainUrl == mainUrl && m_pendingAuxUrl == assistUrl) {
            clearPendingDisplayTarget();
        }
        setMainImage(mainPix);
        setAuxImage(auxPix);
    }

    if (syncButtons) {
        for (XrayImageButton *other : m_tongfangButtons) {
            if (other) {
                other->setSelected(other->typeId() == type);
            }
        }
        for (XrayImageButton *other : m_yisuoButtons) {
            if (other) {
                other->setSelected(other->typeId() == type);
            }
        }
        m_selectedXrayType = type;
    }
    if (emitToggle) {
        emit xrayTypeToggled(type, true);
    }
    qCDebug(lcQcView) << "[BaseView] type" << type << "fallback to xrayImages";
    return true;
}

void QualityControlBaseView::applyOverlaySelection(int index, bool syncButtonSelection)
{
    if (index < 0 || index >= m_overlayItems.size()) {
        return;
    }
    for (int i = 0; i < m_overlayItems.size(); ++i) {
        m_overlayItems[i].setSelected(i == index);
    }
    // Overlay 选择变化后，同步更新主/辅视图，并按需同步底部按钮选中态。
    XrayImage item = m_overlayItems.at(index);
    preloadTaskImageUrls(collectImageUrls(item));
    const QString mainUrl = item.mainXrayImageUrl();
    const QString assistUrl = item.assistXrayImageUrl();
    const int type = imageTypeOf(item);
    m_currentMainUrl = mainUrl;
    m_currentAuxUrl = assistUrl;
    const QPixmap mainPix = resolveTaskPixmap(mainUrl);
    const QPixmap auxPix = resolveTaskPixmap(assistUrl);
    const bool targetReady = isDisplayTargetReady(mainUrl, assistUrl);
    if (m_waitingOriginReady || !targetReady) {
        queuePendingDisplayTarget(type, index, mainUrl, assistUrl);
        if ((!m_mainView || !m_mainView->hasImage()) && !mainPix.isNull()) {
            setMainImage(mainPix);
        }
        if ((!m_auxView || !m_auxView->hasImage()) && !auxPix.isNull()) {
            setAuxImage(auxPix);
        }
        qCDebug(lcQcView) << "[BaseView] overlay target pending index=" << index
                          << "type=" << type
                          << "originGate=" << m_waitingOriginReady
                          << "mainReady=" << isOriginImageReady(mainUrl)
                          << "assistReady=" << isOriginImageReady(assistUrl);
    } else {
        if (m_pendingOverlayIndex == index
            || (m_pendingMainUrl == mainUrl && m_pendingAuxUrl == assistUrl)) {
            clearPendingDisplayTarget();
        }
        setMainImage(mainPix);
        setAuxImage(auxPix);
    }

    if (!syncButtonSelection) {
        return;
    }

    bool hasButton = false;
    for (XrayImageButton *btn : m_tongfangButtons) {
        if (!btn) {
            continue;
        }
        const bool selected = (btn->typeId() == type);
        hasButton = hasButton || selected;
        btn->setSelected(selected);
    }
    for (XrayImageButton *btn : m_yisuoButtons) {
        if (!btn) {
            continue;
        }
        const bool selected = (btn->typeId() == type);
        hasButton = hasButton || selected;
        btn->setSelected(selected);
    }
    m_selectedXrayType = hasButton ? type : -1;
}

void QualityControlBaseView::rebuildOverlayLayerCards()
{
    if (!m_layerView) {
        return;
    }

    QList<LayerViewWidget::LayerItemData> layerItems;
    layerItems.reserve(m_overlayItems.size());
    for (const XrayImage &item : m_overlayItems) {
        LayerViewWidget::LayerItemData data;
        data.name = item.name();
        data.selected = item.isSelected();
        data.image = resolveTaskPixmap(item.mainXrayImageUrl());
        layerItems.append(data);
    }
    m_layerView->setPageSize(3);
    m_layerView->setLayerItems(layerItems);
}

QPixmap QualityControlBaseView::resolveTaskPixmap(const QString &url) const
{
    if (url.isEmpty()) {
        return QPixmap();
    }
    if (m_taskImageCache.contains(url)) {
        return m_taskImageCache.value(url);
    }

    const QPixmap fromProvider = ImageProvider::instance().cached(url);
    if (!fromProvider.isNull()) {
        m_taskImageCache.insert(url, fromProvider);
        return fromProvider;
    }

    if (!isRemoteUrl(url)) {
        const QPixmap local(url);
        if (!local.isNull()) {
            m_taskImageCache.insert(url, local);
        }
        return local;
    }
    return QPixmap();
}

void QualityControlBaseView::preloadTaskImageUrls(const QStringList &urls)
{
    // 仅对远程 URL 触发预加载，本地/资源图直接走同步读取。
    int requestCount = 0;
    for (const QString &url : urls) {
        if (url.isEmpty()) {
            continue;
        }
        if (!resolveTaskPixmap(url).isNull()) {
            continue;
        }
        if (!isRemoteUrl(url)) {
            continue;
        }
        if (m_taskPendingUrls.contains(url)) {
            continue;
        }
        m_taskPendingUrls.insert(url);
        ImageProvider::instance().request(url);
        ++requestCount;
    }
    if (requestCount > 0) {
        qCInfo(lcQcView) << "[BaseView] preload queued remote urls:" << requestCount;
    }
}

QString QualityControlBaseView::originMainUrl() const
{
    for (const XrayImage &item : m_overlayItems) {
        const int type = imageTypeOf(item);
        if (type == XImage_Origin || type == XImage_HM_Origin || type == XImage_YS_Origin || type == XImage_TF_Origin) {
            return item.mainXrayImageUrl();
        }
    }
    if (!m_overlayItems.isEmpty()) {
        return m_overlayItems.first().mainXrayImageUrl();
    }
    return QString();
}

QString QualityControlBaseView::originAssistUrl() const
{
    for (const XrayImage &item : m_overlayItems) {
        const int type = imageTypeOf(item);
        if (type == XImage_Origin || type == XImage_HM_Origin || type == XImage_YS_Origin || type == XImage_TF_Origin) {
            return item.assistXrayImageUrl();
        }
    }
    if (!m_overlayItems.isEmpty()) {
        return m_overlayItems.first().assistXrayImageUrl();
    }
    return QString();
}

int QualityControlBaseView::imageTypeOf(const XrayImage &img)
{
    bool ok = false;
    const int pbType = img.pbEnhancedType().toInt(&ok);
    if (ok) {
        return pbType;
    }
    const QList<int> enhanced = img.enhancedType();
    if (!enhanced.isEmpty()) {
        return enhanced.first();
    }
    return XImage_Invalid;
}

QSet<int> QualityControlBaseView::imageTypesOf(const QList<XrayImage> &images)
{
    QSet<int> types;
    for (const XrayImage &img : images) {
        const int type = imageTypeOf(img);
        if (type != XImage_Invalid) {
            types.insert(type);
        }
    }
    return types;
}

QList<XrayImage> QualityControlBaseView::buildOverlayItems(const ImageDistributeInfo &info)
{
    QList<XrayImage> overlays = info.pbXrayImages;
    if (overlays.isEmpty()) {
        return overlays;
    }

    int originIdx = -1;
    for (int i = 0; i < overlays.size(); ++i) {
        const int type = imageTypeOf(overlays.at(i));
        if (type == XImage_Origin || type == XImage_HM_Origin || type == XImage_YS_Origin || type == XImage_TF_Origin) {
            originIdx = i;
            break;
        }
    }
    if (originIdx > 0) {
        overlays.move(originIdx, 0);
    }
    return overlays;
}

QStringList QualityControlBaseView::collectImageUrls(const XrayImage &img)
{
    QStringList urls;
    // 预下载仅拉取主/辅图，full 图不参与预下载。
    appendUnique(urls, {img.mainXrayImageUrl(), img.assistXrayImageUrl()});
    return urls;
}

void QualityControlBaseView::onImageProviderUpdated(const QString &url, bool success)
{
    if (!m_taskPendingUrls.contains(url)) {
        return;
    }
    m_taskPendingUrls.remove(url);
    qCDebug(lcQcView) << "[BaseView] image updated url=" << url << "success=" << success;

    const QPixmap pix = ImageProvider::instance().cached(url);
    if (!pix.isNull()) {
        m_taskImageCache.insert(url, pix);
    }

    // 原图主图+侧图都到达后才放开“开检/放行”。
    const bool isOriginMain = (!m_taskOriginMainUrl.isEmpty() && url == m_taskOriginMainUrl);
    const bool isOriginAssist = (!m_taskOriginAssistUrl.isEmpty() && url == m_taskOriginAssistUrl);
    if (m_waitingOriginReady && (isOriginMain || isOriginAssist)) {
        if (!success) {
            qCWarning(lcQcView) << "[BaseView] origin image load failed, schedule retry url=" << url;
            QTimer::singleShot(5500, this, [this, url]() {
                if (!m_waitingOriginReady) {
                    return;
                }
                preloadTaskImageUrls(QStringList{url});
            });
        } else {
            qCInfo(lcQcView) << "[BaseView] origin partial ready"
                             << "mainReady=" << isOriginImageReady(m_taskOriginMainUrl)
                             << "assistReady=" << isOriginImageReady(m_taskOriginAssistUrl);
        }
        if (isOriginPairReady()) {
            finishOriginGate(false);
        }
    }

    // 仅当更新 URL 属于当前任务 overlay 时，才重建 Overlay 卡片。
    bool affectsOverlay = false;
    for (const XrayImage &item : m_overlayItems) {
        if (item.mainXrayImageUrl() == url || item.assistXrayImageUrl() == url || item.fullXrayImageUrl() == url) {
            affectsOverlay = true;
            break;
        }
    }
    if (affectsOverlay) {
        rebuildOverlayLayerCards();
    }

    const bool pendingMatched = (!m_pendingMainUrl.isEmpty() && m_pendingMainUrl == url)
            || (!m_pendingAuxUrl.isEmpty() && m_pendingAuxUrl == url);
    if (pendingMatched) {
        tryApplyPendingDisplayTarget();
    } else if (m_currentMainUrl == url || m_currentAuxUrl == url) {
        for (int i = 0; i < m_overlayItems.size(); ++i) {
            if (m_overlayItems.at(i).mainXrayImageUrl() == m_currentMainUrl &&
                m_overlayItems.at(i).assistXrayImageUrl() == m_currentAuxUrl) {
                applyOverlaySelection(i, false);
                break;
            }
        }
    }
}

int QualityControlBaseView::originOverlayIndex() const
{
    for (int i = 0; i < m_overlayItems.size(); ++i) {
        const int type = imageTypeOf(m_overlayItems.at(i));
        if (type == XImage_Origin || type == XImage_HM_Origin
            || type == XImage_YS_Origin || type == XImage_TF_Origin) {
            return i;
        }
    }
    return m_overlayItems.isEmpty() ? -1 : 0;
}

bool QualityControlBaseView::isOriginImageReady(const QString &url) const
{
    if (url.isEmpty()) {
        return true;
    }
    return !resolveTaskPixmap(url).isNull();
}

bool QualityControlBaseView::isOriginPairReady() const
{
    return isOriginImageReady(m_taskOriginMainUrl) && isOriginImageReady(m_taskOriginAssistUrl);
}

void QualityControlBaseView::startOriginGate(int initialIndex)
{
    m_waitingOriginReady = true;
    m_originSubmitBlocked = true;
    m_originInitialIndex = initialIndex;
    clearPendingDisplayTarget();
    setBottomBarTaskActive(m_bottomBarTaskActive);
    if (m_originGateTimer) {
        m_originGateTimer->start(kOriginGateTimeoutMs);
    }
    qCInfo(lcQcView) << "[BaseView] origin gate start timeoutMs=" << kOriginGateTimeoutMs;
}

void QualityControlBaseView::finishOriginGate(bool timeoutFallback)
{
    if (!m_waitingOriginReady && !m_originSubmitBlocked) {
        return;
    }

    m_waitingOriginReady = false;
    m_originSubmitBlocked = false;
    if (m_originGateTimer) {
        m_originGateTimer->stop();
    }

    const bool hasPendingBeforeRelease = (m_pendingType >= 0)
            || (m_pendingOverlayIndex >= 0)
            || !m_pendingMainUrl.isEmpty()
            || !m_pendingAuxUrl.isEmpty();
    const bool shouldForceOrigin = !hasPendingBeforeRelease || isOriginPairReady();
    if (shouldForceOrigin) {
        const int originIndex = originOverlayIndex();
        if (originIndex >= 0) {
            applyOverlaySelection(originIndex, true);
        } else if (m_originInitialIndex >= 0 && m_originInitialIndex < m_overlayItems.size()) {
            applyOverlaySelection(m_originInitialIndex, true);
        }
    } else {
        qCInfo(lcQcView) << "[BaseView] keep user pending target after gate release, skip force origin";
    }

    if (!m_taskDeferredUrls.isEmpty()) {
        const QStringList deferred = QStringList(m_taskDeferredUrls.values());
        m_taskDeferredUrls.clear();
        preloadTaskImageUrls(deferred);
        qCInfo(lcQcView) << "[BaseView] origin gate release, start deferred preload urls:" << deferred.size();
    }

    if (tryApplyPendingDisplayTarget()) {
        qCInfo(lcQcView) << "[BaseView] pending target applied after origin gate release";
    }

    setBottomBarTaskActive(m_bottomBarTaskActive);
    qCInfo(lcQcView) << "[BaseView] origin gate released"
                     << "timeoutFallback=" << timeoutFallback
                     << "mainReady=" << isOriginImageReady(m_taskOriginMainUrl)
                     << "assistReady=" << isOriginImageReady(m_taskOriginAssistUrl);
}

bool QualityControlBaseView::isDisplayTargetReady(const QString &mainUrl, const QString &auxUrl) const
{
    return isOriginImageReady(mainUrl) && isOriginImageReady(auxUrl);
}

void QualityControlBaseView::clearPendingDisplayTarget()
{
    m_pendingType = -1;
    m_pendingOverlayIndex = -1;
    m_pendingMainUrl.clear();
    m_pendingAuxUrl.clear();
}

void QualityControlBaseView::queuePendingDisplayTarget(int type, int overlayIndex,
                                                       const QString &mainUrl, const QString &auxUrl)
{
    m_pendingType = type;
    m_pendingOverlayIndex = overlayIndex;
    m_pendingMainUrl = mainUrl;
    m_pendingAuxUrl = auxUrl;
    qCDebug(lcQcView) << "[BaseView] queue pending target type=" << type
                      << "overlayIndex=" << overlayIndex
                      << "main=" << mainUrl
                      << "assist=" << auxUrl;
}

bool QualityControlBaseView::tryApplyPendingDisplayTarget()
{
    if (m_pendingType < 0 && m_pendingOverlayIndex < 0
        && m_pendingMainUrl.isEmpty() && m_pendingAuxUrl.isEmpty()) {
        return false;
    }
    if (m_waitingOriginReady) {
        return false;
    }
    if (!isDisplayTargetReady(m_pendingMainUrl, m_pendingAuxUrl)) {
        return false;
    }

    const int pendingType = m_pendingType;
    const int overlayIndex = m_pendingOverlayIndex;
    const QString pendingMainUrl = m_pendingMainUrl;
    const QString pendingAuxUrl = m_pendingAuxUrl;
    clearPendingDisplayTarget();

    if (overlayIndex >= 0 && overlayIndex < m_overlayItems.size()) {
        applyOverlaySelection(overlayIndex, true);
        qCInfo(lcQcView) << "[BaseView] apply pending overlay target index=" << overlayIndex
                         << "type=" << pendingType;
        return true;
    }

    const QPixmap mainPix = resolveTaskPixmap(pendingMainUrl);
    const QPixmap auxPix = resolveTaskPixmap(pendingAuxUrl);
    setMainImage(mainPix);
    setAuxImage(auxPix);
    m_currentMainUrl = pendingMainUrl;
    m_currentAuxUrl = pendingAuxUrl;
    qCInfo(lcQcView) << "[BaseView] apply pending fallback target type=" << pendingType;
    return true;
}
