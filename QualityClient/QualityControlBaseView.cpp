#include "QualityControlBaseView.h"

#include "widgets/AnnotationGraphicsView.h"
#include "widgets/CollapsiblePanel.h"
#include "widgets/LayerViewWidget.h"
#include "widgets/ThumbnailWidget.h"
#include "widgets/XrayImageButton.h"
#include "services/qualitycontrolshortcutmanager.h"
#include "Models/XImageType.h"
#include "Models/XrayImage.h"

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
    , m_isYisuoBrand(false)
    , m_shortcutsEnabled(true)
{
}

void QualityControlBaseView::initializeLayout()
{
    QVBoxLayout *rootLayout = new QVBoxLayout;
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    rootLayout->addWidget(buildTopBar());
    rootLayout->addWidget(buildMiddleArea());
    rootLayout->addWidget(buildBottomBar());

    setLayout(rootLayout);
    setMinimumSize(1920, 1080);
    setMaximumSize(1920, 1080);
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
        {XImage_YS_OS, QStringLiteral("OC"), QStringLiteral(":/Images/OC_gray).png"), QStringLiteral(":/Images/OC.png"), QStringLiteral(":/Images/OC_2.png")},
        {XImage_YS_HD, QStringLiteral("HD"), QStringLiteral(":/Images/HD_gray).png"), QStringLiteral(":/Images/HD.png"), QStringLiteral(":/Images/HD_2.png")}
    };
}

void QualityControlBaseView::setBrand(const QString &brandKey)
{
    if (!m_brandStack) {
        return;
    }
    if (brandKey == QStringLiteral("yisuo")) {
        m_brandStack->setCurrentIndex(1);
        m_isYisuoBrand = true;
    } else {
        m_brandStack->setCurrentIndex(0);
        m_isYisuoBrand = false;
    }
    clearXraySelection();
    if (m_shortcutManager) {
        m_shortcutManager->setBrandIsYisuo(m_isYisuoBrand);
    }
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
    for (auto it = m_xrayButtons.begin(); it != m_xrayButtons.end(); ++it) {
        const bool available = types.contains(it.key());
        it.value()->setAvailable(available);
        if (!available && it.value()->isSelected() && m_selectedXrayType == it.key()) {
            m_selectedXrayType = -1;
        }
    }
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
    QWidget *middle = new QWidget(this);
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

    connect(m_layerView, &LayerViewWidget::xrayItemSelected, this, [this](const XrayImage &item) {
        if (!item.mainXrayImageUrl().isEmpty()) {
            setMainImage(QPixmap(item.mainXrayImageUrl()));
        }
        if (!item.assistXrayImageUrl().isEmpty()) {
            setAuxImage(QPixmap(item.assistXrayImageUrl()));
        }
    });

    connect(m_overlayPanel, &CollapsiblePanel::collapsedChanged, this, [this]() {
        updateOverlayGeometry();
    });
    updateOverlayGeometry();
    return middle;
}

QWidget *QualityControlBaseView::buildBottomBar()
{
    QWidget *bottom = new QWidget(this);
    bottom->setMinimumHeight(127);
    bottom->setMaximumHeight(127);
    bottom->setStyleSheet(QStringLiteral("background:#ffffff;"));

    QHBoxLayout *layout = new QHBoxLayout(bottom);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(15);

    m_thumbnail = new ThumbnailWidget(bottom);
    m_thumbnail->setFixedSize(180, 100);
    m_thumbnail->setStyleSheet(QStringLiteral("border:1px solid #c0c0c0;"));

    QWidget *tools = new QWidget(bottom);
    tools->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QHBoxLayout *toolsLayout = new QHBoxLayout(tools);
    toolsLayout->setContentsMargins(0, 0, 0, 0);
    toolsLayout->setSpacing(6);

    m_brandStack = new QStackedWidget(tools);
    m_brandStack->addWidget(buildBrandButtons(tongfangSpecs(), &m_tongfangButtons));
    m_brandStack->addWidget(buildBrandButtons(yisuoSpecs(), &m_yisuoButtons));
    m_brandStack->setCurrentIndex(0);

    toolsLayout->addWidget(m_brandStack);

    layout->addWidget(m_thumbnail);
    layout->addWidget(tools);

    QWidget *rightPanel = new QWidget(bottom);
    rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QHBoxLayout *rightLayout = new QHBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(12);
    rightLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QWidget *actionPanel = new QWidget(rightPanel);
    QVBoxLayout *actionLayout = new QVBoxLayout(actionPanel);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(6);

    auto makeActionButton = [actionPanel](const QString &text, const QString &style) {
        QPushButton *button = new QPushButton(text, actionPanel);
        button->setMinimumSize(120, 36);
        button->setStyleSheet(style);
        return button;
    };

    QWidget *actionRow1 = new QWidget(actionPanel);
    QHBoxLayout *actionRow1Layout = new QHBoxLayout(actionRow1);
    actionRow1Layout->setContentsMargins(0, 0, 0, 0);
    actionRow1Layout->setSpacing(6);
    m_startCheckButton = makeActionButton(QStringLiteral("开检"),
        QStringLiteral("QPushButton{background:#c53a2b;color:white;border-radius:6px;padding:6px 16px;font-weight:bold;}"));
    m_passButton = makeActionButton(QStringLiteral("放行"),
        QStringLiteral("QPushButton{background:#2dbb63;color:white;border-radius:6px;padding:6px 16px;font-weight:bold;}"));
    actionRow1Layout->addWidget(m_startCheckButton);
    actionRow1Layout->addWidget(m_passButton);

    QWidget *actionRow2 = new QWidget(actionPanel);
    QHBoxLayout *actionRow2Layout = new QHBoxLayout(actionRow2);
    actionRow2Layout->setContentsMargins(0, 0, 0, 0);
    actionRow2Layout->setSpacing(6);
    m_detailButton = makeActionButton(QStringLiteral("人包详情"),
        QStringLiteral("QPushButton{background:#27b4e6;color:white;border-radius:6px;padding:6px 16px;}"));
    m_aiToggleButton = makeActionButton(QStringLiteral("隐藏/显示AI"),
        QStringLiteral("QPushButton{background:#80b7d8;color:white;border-radius:6px;padding:6px 16px;}"));
    actionRow2Layout->addWidget(m_detailButton);
    actionRow2Layout->addWidget(m_aiToggleButton);

    actionLayout->addWidget(actionRow1);
    actionLayout->addWidget(actionRow2);

    QWidget *infoPanel = new QWidget(rightPanel);
    QGridLayout *infoLayout = new QGridLayout(infoPanel);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setHorizontalSpacing(8);
    infoLayout->setVerticalSpacing(6);

    auto makeInfoValue = [infoPanel](const QString &text) {
        QLabel *label = new QLabel(text, infoPanel);
        label->setStyleSheet(QStringLiteral("QLabel{background:#f1f1f1;border-radius:12px;padding:4px 10px;}"));
        return label;
    };

    QLabel *judgeTitle = new QLabel(QStringLiteral("判图结论："), infoPanel);
    m_judgeValueLabel = makeInfoValue(QStringLiteral("放行"));
    QLabel *timeTitle = new QLabel(QStringLiteral("时间："), infoPanel);
    m_timeValueLabel = makeInfoValue(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));

    QLabel *sharpTitle = new QLabel(QStringLiteral("图像锐度："), infoPanel);
    m_freshnessValueLabel = makeInfoValue(QStringLiteral("--"));
    QLabel *channelTitle = new QLabel(QStringLiteral("通道："), infoPanel);
    m_channelValueLabel = makeInfoValue(QStringLiteral("8"));

    infoLayout->addWidget(judgeTitle, 0, 0);
    infoLayout->addWidget(m_judgeValueLabel, 0, 1);
    infoLayout->addWidget(timeTitle, 0, 2);
    infoLayout->addWidget(m_timeValueLabel, 0, 3);
    infoLayout->addWidget(sharpTitle, 1, 0);
    infoLayout->addWidget(m_freshnessValueLabel, 1, 1);
    infoLayout->addWidget(channelTitle, 1, 2);
    infoLayout->addWidget(m_channelValueLabel, 1, 3);

    QWidget *badge = new QWidget(rightPanel);
    QHBoxLayout *badgeLayout = new QHBoxLayout(badge);
    badgeLayout->setContentsMargins(6, 0, 6, 0);
    badgeLayout->setSpacing(6);
    QLabel *badgeIcon = new QLabel(badge);
    badgeIcon->setFixedSize(44, 44);
    badgeIcon->setAlignment(Qt::AlignCenter);
    badgeIcon->setStyleSheet(QStringLiteral("QLabel{background:#e9f0ff;border-radius:22px;}"));
    badgeIcon->setPixmap(QPixmap(QStringLiteral(":/Images/robot.png")).scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QLabel *badgeText = new QLabel(QStringLiteral("首检图像"), badge);
    badgeText->setStyleSheet(QStringLiteral("QLabel{font-weight:bold;}"));
    badgeLayout->addWidget(badgeIcon);
    badgeLayout->addWidget(badgeText);

    rightLayout->addWidget(actionPanel);
    rightLayout->addWidget(infoPanel);
    rightLayout->addWidget(badge);

    layout->addWidget(rightPanel, 1);
    layout->setStretch(0, 0);
    layout->setStretch(1, 0);
    layout->setStretch(2, 1);

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
        m_shortcutManager->configure(typeButtons, m_escButton, m_startCheckButton, m_passButton, m_detailButton);
        m_shortcutManager->setBrandIsYisuo(m_isYisuoBrand);
    }

    return bottom;
}

QWidget *QualityControlBaseView::buildBrandButtons(const QList<XrayButtonSpec> &specs, QList<XrayImageButton*> *buttonList)
{
    QWidget *container = new QWidget(this);
    QGridLayout *layout = new QGridLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setHorizontalSpacing(8);
    layout->setVerticalSpacing(6);

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

    m_escButton = makeIconButton(QStringLiteral(":/Images/esc_origin.png"), QStringLiteral("ESC"));
    m_zoomInButton = makeIconButton(QStringLiteral(":/Images/tf_big.png"), QStringLiteral("放大"));
    m_zoomOutButton = makeIconButton(QStringLiteral(":/Images/tf_small.png"), QStringLiteral("缩小"));

    connect(m_escButton, &QPushButton::clicked, this, [this]() {
        if (m_activeView) {
            m_activeView->resetZoom();
        }
    });
    connect(m_zoomInButton, &QPushButton::clicked, this, [this]() {
        if (m_activeView) {
            m_activeView->zoomIn();
        }
    });
    connect(m_zoomOutButton, &QPushButton::clicked, this, [this]() {
        if (m_activeView) {
            m_activeView->zoomOut();
        }
    });

    layout->addWidget(m_escButton, 1, 1);
    layout->addWidget(m_zoomInButton, 1, 2);
    layout->addWidget(m_zoomOutButton, 1, 3);
    return container;
}

void QualityControlBaseView::handleXrayButtonClicked(XrayImageButton *button)
{
    if (!button) {
        return;
    }
    if (!button->isAvailable()) {
        emit xrayTypeUnavailableClicked(button->typeId());
        return;
    }

    if (button->isSelected()) {
        button->setSelected(false);
        if (m_selectedXrayType == button->typeId()) {
            m_selectedXrayType = -1;
        }
        emit xrayTypeToggled(button->typeId(), false);
        return;
    }

    const bool isTongfang = m_tongfangButtons.contains(button);
    const QList<XrayImageButton*> &group = isTongfang ? m_tongfangButtons : m_yisuoButtons;
    for (XrayImageButton *other : group) {
        if (other != button && other->isSelected()) {
            other->setSelected(false);
            emit xrayTypeToggled(other->typeId(), false);
        }
    }
    button->setSelected(true);
    m_selectedXrayType = button->typeId();
    emit xrayTypeToggled(button->typeId(), true);
}

void QualityControlBaseView::setShortcutsEnabled(bool enabled)
{
    m_shortcutsEnabled = enabled;
    if (m_shortcutManager) {
        m_shortcutManager->setEnabled(enabled);
    }
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
