#include "QualityControlView.h"

#include "widgets/SwitchButton.h"
#include "widgets/LayerViewWidget.h"
#include "Models/XrayImage.h"
#include "services/qualitycontrolservice.h"
#include "services/qualitycontrolcontroller.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QDateTime>
#include <QFont>

QualityControlView::QualityControlView(QWidget *parent)
    : QualityControlBaseView(parent)
    , m_taskCountLabel(nullptr)
    , m_durationTitleLabel(nullptr)
    , m_durationDot(nullptr)
    , m_durationValueLabel(nullptr)
    , m_userLabel(nullptr)
    , m_pauseToggle(nullptr)
    , m_statsButton(nullptr)
    , m_historyButton(nullptr)
    , m_switchModeButton(nullptr)
    , m_testIndex(0)
    , m_durationTimer(nullptr)
    , m_elapsedSeconds(0)
    , m_service(new QualityControlService(this))
    , m_controller(new QualityControlController(this))
{
    initializeLayout();
    m_testImages << QStringLiteral(":/Images/yisuoTest.png")
                 << QStringLiteral(":/Images/yisuoTest1.png")
                 << QStringLiteral(":/Images/yisuoTest2.png")
                 << QStringLiteral(":/Images/yisuoTest3.png");
    initTestLayerItems();
    loadNextImage();

    connect(m_startCheckButton, &QPushButton::clicked, m_service, &QualityControlService::startCheck);
    connect(m_passButton, &QPushButton::clicked, m_service, &QualityControlService::pass);
    connect(m_detailButton, &QPushButton::clicked, m_service, &QualityControlService::openPersonDetail);
    connect(this, &QualityControlBaseView::xrayTypeToggled, m_service, &QualityControlService::switchXrayType);

    connect(m_service, &QualityControlService::availableTypesReceived, m_controller, &QualityControlController::setAvailableTypes);
    connect(m_service, &QualityControlService::mainImageReceived, m_controller, &QualityControlController::setMainImage);
    connect(m_service, &QualityControlService::auxImageReceived, m_controller, &QualityControlController::setAuxImage);
    connect(m_service, &QualityControlService::judgeResultReceived, m_controller, &QualityControlController::setJudgeResultText);
    connect(m_service, &QualityControlService::freshnessReceived, m_controller, &QualityControlController::setFreshnessText);
    connect(m_service, &QualityControlService::channelReceived, m_controller, &QualityControlController::setChannelText);
    connect(m_service, &QualityControlService::timeTextReceived, m_controller, &QualityControlController::setTimeText);

    connect(m_controller, &QualityControlController::availableTypesChanged, this, &QualityControlBaseView::setAvailableXrayTypes);
    connect(m_controller, &QualityControlController::mainImageChanged, this, &QualityControlBaseView::setMainImage);
    connect(m_controller, &QualityControlController::auxImageChanged, this, &QualityControlBaseView::setAuxImage);
    connect(m_controller, &QualityControlController::judgeResultChanged, this, &QualityControlBaseView::setJudgeResultText);
    connect(m_controller, &QualityControlController::freshnessChanged, this, &QualityControlBaseView::setFreshnessText);
    connect(m_controller, &QualityControlController::channelChanged, this, &QualityControlBaseView::setChannelText);
    connect(m_controller, &QualityControlController::timeTextChanged, this, &QualityControlBaseView::setTimeText);
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

void QualityControlView::setMainImage(const QPixmap &pixmap)
{
    QualityControlBaseView::setMainImage(pixmap);
}

void QualityControlView::setAuxImage(const QPixmap &pixmap)
{
    QualityControlBaseView::setAuxImage(pixmap);
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
    if (m_timeValueLabel) {
        m_timeValueLabel->setText(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    }
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

void QualityControlView::initTestLayerItems()
{
    if (!m_layerView) {
        return;
    }
    QList<XrayImage> items;
    auto makeItem = [this](const QString &mainPath, const QString &assistPath, const QString &pbType, qint64 viewCount, bool selected) {
        XrayImage img;
        img.setMainXrayImageUrl(mainPath);
        img.setAssistXrayImageUrl(assistPath);
        if (!pbType.isEmpty()) {
            img.setPbEnhancedType(pbType);
        }
        if (viewCount > 0) {
            img.setViewCount(viewCount);
        }
        img.setSelected(selected);
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
}
