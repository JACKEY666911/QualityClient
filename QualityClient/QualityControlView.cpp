#include "QualityControlView.h"
#include "widgets/AnnotationGraphicsView.h"
#include "widgets/CollapsiblePanel.h"
#include "widgets/LayerViewWidget.h"
#include "widgets/SwitchButton.h"
#include "widgets/ThumbnailWidget.h"
#include "Models/XrayImage.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QPushButton>
#include <QPixmap>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QStringLiteral>
QualityControlView::QualityControlView(QWidget *parent)
    : QWidget(parent)
    , m_taskCountLabel(nullptr)
    , m_countdownLabel(nullptr)
    , m_durationTitleLabel(nullptr)
    , m_durationDot(nullptr)
    , m_durationValueLabel(nullptr)
    , m_userLabel(nullptr)
    , m_pauseToggle(nullptr)
    , m_statsButton(nullptr)
    , m_historyButton(nullptr)
    , m_switchModeButton(nullptr)
    , m_mainView(nullptr)
    , m_auxView(nullptr)
    , m_activeView(nullptr)
    , m_brandStack(nullptr)
    , m_thumbnail(nullptr)
    , m_middleArea(nullptr)
    , m_overlayPanel(nullptr)
    , m_layerView(nullptr)
    , m_testIndex(0)
    , m_durationTimer(nullptr)
    , m_elapsedSeconds(0)
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

    m_testImages << QStringLiteral(":/Images/yisuoTest.png")
                 << QStringLiteral(":/Images/yisuoTest1.png")
                 << QStringLiteral(":/Images/yisuoTest2.png")
                 << QStringLiteral(":/Images/yisuoTest3.png");
    loadNextImage();
}

void QualityControlView::setUserName(const QString &userName)
{
    if (m_userLabel) {
        m_userLabel->setText(QStringLiteral("用户：%1").arg(userName));
    }
}

void QualityControlView::setTaskCount(int count)
{
    if (m_taskCountLabel) {
        m_taskCountLabel->setText(QStringLiteral("任务数量：%1").arg(count));
    }
}

void QualityControlView::setCountdownText(const QString &text)
{
    if (m_durationValueLabel) {
        m_durationValueLabel->setText(text);
    }
}

void QualityControlView::setBrand(const QString &brandKey)
{
    if (!m_brandStack) {
        return;
    }
    if (brandKey == QStringLiteral("yisuo")) {
        m_brandStack->setCurrentIndex(1);
    } else {
        m_brandStack->setCurrentIndex(0);
    }
}

void QualityControlView::setMainImage(const QPixmap &pixmap)
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

void QualityControlView::setAuxImage(const QPixmap &pixmap)
{
    if (m_auxView) {
        m_auxView->setImage(pixmap);
    }
    if (m_thumbnail && m_activeView == m_auxView) {
        m_thumbnail->setImage(pixmap);
    }
}

void QualityControlView::loadNextImage()
{
    if (m_testImages.isEmpty()) {
        return;
    }
    const QString path = m_testImages.at(m_testIndex % m_testImages.size());
    m_testIndex++;

    QPixmap pix(path);
    setMainImage(pix);
    setAuxImage(pix);

    if (m_mainView) {
        m_mainView->clearAnnotations();
        m_mainView->resetZoom();
    }
    if (m_auxView) {
        m_auxView->clearAnnotations();
        m_auxView->resetZoom();
    }
}

QList<AnnotationGraphicsView::AnnotationData> QualityControlView::mainAnnotations() const
{
    if (!m_mainView) {
        return {};
    }
    return m_mainView->annotations();
}

QList<AnnotationGraphicsView::AnnotationData> QualityControlView::auxAnnotations() const
{
    if (!m_auxView) {
        return {};
    }
    return m_auxView->annotations();
}


void QualityControlView::handleViewActivated(AnnotationGraphicsView *view)
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

void QualityControlView::handlePauseToggled(bool checked)
{
    if (m_durationTimer) {
        if (checked) {
            m_durationTimer->start();
        } else {
            m_durationTimer->stop();
        }
    }
    emit taskPauseToggled(checked);
}

void QualityControlView::handleDurationTick()
{
    ++m_elapsedSeconds;
    int hours = m_elapsedSeconds / 3600;
    int minutes = (m_elapsedSeconds % 3600) / 60;
    int seconds = m_elapsedSeconds % 60;
    QString text = QStringLiteral("%1:%2:%3")
                       .arg(hours, 2, 10, QLatin1Char('0'))
                       .arg(minutes, 2, 10, QLatin1Char('0'))
                       .arg(seconds, 2, 10, QLatin1Char('0'));
    if (m_durationValueLabel) {
        m_durationValueLabel->setText(text);
    }
}

bool QualityControlView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_middleArea && event->type() == QEvent::Resize) {
        updateOverlayGeometry();
    }
    return QWidget::eventFilter(watched, event);
}

QWidget *QualityControlView::buildTopBar()
{
    QWidget *topBar = new QWidget(this);
    topBar->setMinimumHeight(55);
    topBar->setMaximumHeight(55);
    topBar->setObjectName(QStringLiteral("qcTopBar"));
    topBar->setStyleSheet(QStringLiteral(
        "QWidget#qcTopBar{background:#117595;}"
        "QLabel{color:white;font-family:\"Microsoft YaHei\";}"
        "QPushButton{background:#01b9ec;color:white;border-radius:4px;padding:4px 12px;font-family:\"Microsoft YaHei\";}"
    ));

    QHBoxLayout *layout = new QHBoxLayout(topBar);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(12);

    QLabel *logo = new QLabel(topBar);
    logo->setPixmap(QPixmap(QStringLiteral(":/Images/logo.png")));
    logo->setFixedSize(120, 40);
    logo->setScaledContents(true);

    QLabel *title = new QLabel(QStringLiteral("般睿安检集中质控系统"), topBar);
    QFont titleFont;
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);

    m_taskCountLabel = new QLabel(QStringLiteral("任务数量：0"), topBar);

    m_pauseToggle = new SwitchButton(topBar);
    m_pauseToggle->setChecked(true);
    connect(m_pauseToggle, &QAbstractButton::toggled, this, &QualityControlView::handlePauseToggled);

    m_durationDot = new QLabel(topBar);
    m_durationDot->setFixedSize(10, 10);
    m_durationDot->setStyleSheet(QStringLiteral("background:#43d86c;border-radius:5px;"));

    m_durationTitleLabel = new QLabel(QStringLiteral("时长："), topBar);
    m_durationValueLabel = new QLabel(QStringLiteral("00:00:00"), topBar);

    m_statsButton = new QPushButton(QStringLiteral("查看任务统计"), topBar);
    connect(m_statsButton, &QPushButton::clicked, this, &QualityControlView::requestTaskStats);

    m_historyButton = new QPushButton(QStringLiteral("质控记录回查"), topBar);
    connect(m_historyButton, &QPushButton::clicked, this, &QualityControlView::requestHistory);

    m_userLabel = new QLabel(QStringLiteral("用户：--"), topBar);

    m_switchModeButton = new QPushButton(QStringLiteral("切换质控模式"), topBar);
    connect(m_switchModeButton, &QPushButton::clicked, this, &QualityControlView::requestModeSwitch);

    layout->addWidget(logo);
    layout->addWidget(title);
    layout->addWidget(m_taskCountLabel);
    layout->addWidget(m_pauseToggle);
    layout->addStretch(1);
    layout->addWidget(m_durationDot);
    layout->addWidget(m_durationTitleLabel);
    layout->addWidget(m_durationValueLabel);
    layout->addStretch(1);
    layout->addWidget(m_statsButton);
    layout->addWidget(m_historyButton);
    layout->addWidget(m_userLabel);
    layout->addWidget(m_switchModeButton);

    if (!m_durationTimer) {
        m_durationTimer = new QTimer(this);
        m_durationTimer->setInterval(1000);
        connect(m_durationTimer, &QTimer::timeout, this, &QualityControlView::handleDurationTick);
        m_durationTimer->start();
    }

    return topBar;
}

QWidget *QualityControlView::buildMiddleArea()
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

    connect(m_mainView, &AnnotationGraphicsView::activated, this, &QualityControlView::handleViewActivated);
    connect(m_auxView, &AnnotationGraphicsView::activated, this, &QualityControlView::handleViewActivated);

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

    connect(m_layerView, &LayerViewWidget::xrayItemSelected, this, [this](XrayImage *item) {
        if (!item) {
            return;
        }
        if (!item->mainXrayImageUrl().isEmpty()) {
            setMainImage(QPixmap(item->mainXrayImageUrl()));
        }
        if (!item->assistXrayImageUrl().isEmpty()) {
            setAuxImage(QPixmap(item->assistXrayImageUrl()));
        }
    });

    QList<XrayImage*> items;
    auto makeItem = [this](const QString &mainPath, const QString &assistPath, const QString &pbType, qint64 viewCount, bool selected) {
        XrayImage *img = new XrayImage(m_layerView);
        img->setMainXrayImageUrl(mainPath);
        img->setAssistXrayImageUrl(assistPath);
        if (!pbType.isEmpty()) {
            img->setPbEnhancedType(pbType);
        }
        if (viewCount > 0) {
            img->setViewCount(viewCount);
        }
        img->setSelected(selected);
        return img;
    };

    items.append(makeItem(QStringLiteral(":/Images/yisuoTest.png"), QStringLiteral(":/Images/yisuoTest1.png"), QString(), 1, true));
    items.append(makeItem(QStringLiteral(":/Images/yisuoTest1.png"), QStringLiteral(":/Images/yisuoTest2.png"), QString(), 2, false));
    items.append(makeItem(QStringLiteral(":/Images/yisuoTest2.png"), QStringLiteral(":/Images/yisuoTest3.png"), QStringLiteral("801"), 1, false));
    items.append(makeItem(QStringLiteral(":/Images/yisuoTest3.png"), QStringLiteral(":/Images/yisuoTest.png"), QString(), 3, false));
    items.append(makeItem(QStringLiteral(":/Images/yisuoTest.png"), QStringLiteral(":/Images/yisuoTest2.png"), QString(), 1, false));
    items.append(makeItem(QStringLiteral(":/Images/yisuoTest1.png"), QStringLiteral(":/Images/yisuoTest3.png"), QStringLiteral("802"), 1, false));
    items.append(makeItem(QStringLiteral(":/Images/yisuoTest2.png"), QStringLiteral(":/Images/yisuoTest.png"), QString(), 2, false));

    m_layerView->setPageSize(3);
    m_layerView->setXrayItems(items);
    m_layerView->setDurationText(QStringLiteral("10秒"));
    connect(m_overlayPanel, &CollapsiblePanel::collapsedChanged, this, [this]() {
        updateOverlayGeometry();
    });
    updateOverlayGeometry();
    return middle;
}

QWidget *QualityControlView::buildBottomBar()
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
    QHBoxLayout *toolsLayout = new QHBoxLayout(tools);
    toolsLayout->setContentsMargins(0, 0, 0, 0);
    toolsLayout->setSpacing(10);

    m_brandStack = new QStackedWidget(tools);
    QStringList tongfang = {QStringLiteral("ED"), QStringLiteral("GEN"), QStringLiteral("HI"),
                            QStringLiteral("LOW"), QStringLiteral("OS"), QStringLiteral("MS")};
    QStringList yisuo = {QStringLiteral("E0"), QStringLiteral("E1"), QStringLiteral("E2"),
                         QStringLiteral("SC"), QStringLiteral("OC"), QStringLiteral("HD")};
    m_brandStack->addWidget(buildBrandButtons(tongfang));
    m_brandStack->addWidget(buildBrandButtons(yisuo));
    m_brandStack->setCurrentIndex(0);

    QPushButton *resetButton = new QPushButton(QStringLiteral("ESC"), tools);
    QPushButton *zoomInButton = new QPushButton(QStringLiteral("放大"), tools);
    QPushButton *zoomOutButton = new QPushButton(QStringLiteral("缩小"), tools);
    QPushButton *manualToggle = new QPushButton(QStringLiteral("手工标注"), tools);
    QPushButton *aiToggle = new QPushButton(QStringLiteral("AI标注"), tools);
    QPushButton *nextButtonA = new QPushButton(QStringLiteral("下一张"), tools);
    QPushButton *nextButtonB = new QPushButton(QStringLiteral("下一张"), tools);
    manualToggle->setCheckable(true);
    aiToggle->setCheckable(true);
    manualToggle->setChecked(true);
    aiToggle->setChecked(true);

    connect(resetButton, &QPushButton::clicked, this, [this]() {
        if (m_activeView) {
            m_activeView->resetZoom();
        }
    });
    connect(zoomInButton, &QPushButton::clicked, this, [this]() {
        if (m_activeView) {
            m_activeView->zoomIn();
        }
    });
    connect(zoomOutButton, &QPushButton::clicked, this, [this]() {
        if (m_activeView) {
            m_activeView->zoomOut();
        }
    });
    connect(manualToggle, &QPushButton::toggled, this, [this](bool checked) {
        if (m_mainView) {
            m_mainView->setManualVisible(checked);
        }
        if (m_auxView) {
            m_auxView->setManualVisible(checked);
        }
    });
    connect(aiToggle, &QPushButton::toggled, this, [this](bool checked) {
        if (m_mainView) {
            m_mainView->setAiVisible(checked);
        }
        if (m_auxView) {
            m_auxView->setAiVisible(checked);
        }
    });
    connect(nextButtonA, &QPushButton::clicked, this, [this]() { loadNextImage(); });
    connect(nextButtonB, &QPushButton::clicked, this, [this]() { loadNextImage(); });

    toolsLayout->addWidget(m_brandStack);
    toolsLayout->addWidget(resetButton);
    toolsLayout->addWidget(zoomInButton);
    toolsLayout->addWidget(zoomOutButton);
    toolsLayout->addWidget(manualToggle);
    toolsLayout->addWidget(aiToggle);
    toolsLayout->addWidget(nextButtonA);
    toolsLayout->addWidget(nextButtonB);
    toolsLayout->addStretch(1);

    layout->addWidget(m_thumbnail);
    layout->addWidget(tools, 1);

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

    return bottom;
}

QWidget *QualityControlView::buildBrandButtons(const QStringList &labels)
{
    QWidget *container = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    for (const QString &label : labels) {
        QPushButton *btn = new QPushButton(label, container);
        btn->setFixedSize(60, 32);
        layout->addWidget(btn);
    }
    return container;
}

void QualityControlView::positionOverlayPanel()
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

void QualityControlView::updateOverlayGeometry()
{
    positionOverlayPanel();
}
