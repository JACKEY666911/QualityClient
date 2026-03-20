#include "QualityControlHistoryView.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "services/qualitycontrolhistoryservice.h"
#include "services/qualitycontrolcontroller.h"

QualityControlHistoryView::QualityControlHistoryView(QWidget *parent)
    : QualityControlBaseView(parent)
    , m_backButton(nullptr)
    , m_titleLabel(nullptr)
    , m_service(new QualityControlHistoryService(this))
    , m_controller(new QualityControlController(this))
{
    initializeLayout();

    connect(m_startCheckButton, &QPushButton::clicked, m_service, &QualityControlHistoryService::startCheck);
    connect(m_passButton, &QPushButton::clicked, m_service, &QualityControlHistoryService::pass);
    connect(m_detailButton, &QPushButton::clicked, m_service, &QualityControlHistoryService::openPersonDetail);
    connect(this, &QualityControlBaseView::xrayTypeToggled, m_service, &QualityControlHistoryService::switchXrayType);

    connect(m_service, &QualityControlHistoryService::availableTypesReceived, m_controller, &QualityControlController::setAvailableTypes);
    connect(m_service, &QualityControlHistoryService::mainImageReceived, m_controller, &QualityControlController::setMainImage);
    connect(m_service, &QualityControlHistoryService::auxImageReceived, m_controller, &QualityControlController::setAuxImage);
    connect(m_service, &QualityControlHistoryService::judgeResultReceived, m_controller, &QualityControlController::setJudgeResultText);
    connect(m_service, &QualityControlHistoryService::freshnessReceived, m_controller, &QualityControlController::setFreshnessText);
    connect(m_service, &QualityControlHistoryService::channelReceived, m_controller, &QualityControlController::setChannelText);
    connect(m_service, &QualityControlHistoryService::timeTextReceived, m_controller, &QualityControlController::setTimeText);

    connect(m_controller, &QualityControlController::availableTypesChanged, this, &QualityControlBaseView::setAvailableXrayTypes);
    connect(m_controller, &QualityControlController::mainImageChanged, this, &QualityControlBaseView::setMainImage);
    connect(m_controller, &QualityControlController::auxImageChanged, this, &QualityControlBaseView::setAuxImage);
    connect(m_controller, &QualityControlController::judgeResultChanged, this, &QualityControlBaseView::setJudgeResultText);
    connect(m_controller, &QualityControlController::freshnessChanged, this, &QualityControlBaseView::setFreshnessText);
    connect(m_controller, &QualityControlController::channelChanged, this, &QualityControlBaseView::setChannelText);
    connect(m_controller, &QualityControlController::timeTextChanged, this, &QualityControlBaseView::setTimeText);
}

QWidget *QualityControlHistoryView::buildTopBar()
{
    QWidget *topBar = new QWidget(this);
    topBar->setMinimumHeight(55);
    topBar->setMaximumHeight(55);
    topBar->setObjectName(QStringLiteral("qcHistoryTopBar"));
    topBar->setStyleSheet(QStringLiteral(
        "QWidget#qcHistoryTopBar{background:#117595;}"
        "QLabel{color:white;font-family:\"Microsoft YaHei\";}"
        "QPushButton{background:#01b9ec;color:white;border-radius:4px;padding:4px 12px;font-family:\"Microsoft YaHei\";}"
    ));

    QHBoxLayout *layout = new QHBoxLayout(topBar);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(12);

    m_backButton = new QPushButton(QStringLiteral("返回"), topBar);
    connect(m_backButton, &QPushButton::clicked, this, &QualityControlHistoryView::requestBack);

    m_titleLabel = new QLabel(QStringLiteral("质控记录回查"), topBar);
    QFont titleFont;
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);

    layout->addWidget(m_backButton);
    layout->addWidget(m_titleLabel);
    layout->addStretch(1);

    return topBar;
}
