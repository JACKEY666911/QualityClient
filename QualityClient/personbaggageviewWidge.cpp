#include "personbaggageviewWidge.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QStyleOption>
#include <QPainter>
#include <QSpacerItem>

// ===================== 构造函数 & 核心初始化 =====================
PersonBaggageViewWidget::PersonBaggageViewWidget(QWidget *parent)
    : QWidget(parent)
{
    // 全局样式初始化
    setStyleSheet(R"(
        QWidget {
            background-color: #f5f5f5;
            font-family: "Microsoft YaHei";
        }
        QFrame {
            background-color: white;
            border-radius: 8px;
            padding: 15px;
        }
        QLabel {
            font-size: 14px;
            color: #333333;
            margin: 3px 0;
        }
        QLabel#titleLabel {
            font-size: 18px;
            font-weight: bold;
            color: #0088cc;
            margin-bottom: 15px;
        }
        QLabel.pass { color: green; }
        QLabel.suspicious { color: red; }
        QPushButton {
            background-color: #f0f0f0;
            border: none;
            border-radius: 3px;
            padding: 3px 8px;
        }
        QPushButton:hover { background-color: #e0e0e0; }
        QPushButton#cameraBtn {
            background-color: transparent;
            font-size: 20px;
            padding: 0;
        }
    )");

    // 初始化所有UI组件
    initUI();
}

// ===================== 整体UI初始化 =====================
void PersonBaggageViewWidget::initUI()
{
    // 主布局（垂直）
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    // 1. 顶部导航栏
    initTopNavigationBar();
    mainLayout->addWidget(m_topBar);

    // 2. 第一行：安检信息 + 质控信息
    QHBoxLayout *row1Layout = new QHBoxLayout();
    row1Layout->setSpacing(15);

    // 安检信息卡片（占1份）
    initSecurityCheckCard();
    row1Layout->addWidget(m_securityCard, 1);

    // 质控信息卡片（占2份）
    initQualityControlCard();
    row1Layout->addWidget(m_qualityCard, 2);

    mainLayout->addLayout(row1Layout);

    // 3. 第二行：人包对应信息
    initPersonBaggageCard();
    mainLayout->addWidget(m_baggageCard);
}

// ===================== 顶部导航栏初始化 =====================
void PersonBaggageViewWidget::initTopNavigationBar()
{
    m_topBar = new QWidget(this);
    m_topBar->setFixedHeight(40);
    m_topBar->setStyleSheet(R"(
        QWidget {
            background-color: #0088cc;
            color: white;
            font-family: "Microsoft YaHei";
        }
        QPushButton {
            background-color: #00a0e9;
            border: none;
            border-radius: 3px;
            padding: 5px 10px;
            color: white;
        }
        QPushButton:hover { background-color: #0090d9; }
    )");

    QHBoxLayout *layout = new QHBoxLayout(m_topBar);
    layout->setContentsMargins(15, 0, 15, 0);
    layout->setSpacing(15);

    // Logo标签
    QLabel *logoLabel = new QLabel("LOGO", m_topBar);
    QFont logoFont = logoLabel->font();
    logoFont.setBold(true);
    logoFont.setPointSize(16);
    logoLabel->setFont(logoFont);
    layout->addWidget(logoLabel);

    // 标题标签
    QLabel *titleLabel = new QLabel("殷睿集中质控管理平台", m_topBar);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);

    // 拉伸项：右侧控件右对齐
    layout->addStretch();

    // 质控记录回查按钮
    QPushButton *recordBtn = new QPushButton("质控记录回查", m_topBar);
    layout->addWidget(recordBtn);

    // 管理员标签
    QLabel *adminLabel = new QLabel("admin", m_topBar);
    QFont adminFont = adminLabel->font();
    adminFont.setPointSize(14);
    adminLabel->setFont(adminFont);
    layout->addWidget(adminLabel);

    // 菜单按钮
    QPushButton *menuBtn = new QPushButton("☰", m_topBar);
    menuBtn->setFixedSize(30, 30);
    layout->addWidget(menuBtn);
}

// ===================== 安检信息卡片初始化 =====================
void PersonBaggageViewWidget::initSecurityCheckCard()
{
    m_securityCard = new QFrame(this);

    QHBoxLayout *mainLayout = new QHBoxLayout(m_securityCard);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(20);

    // 左侧信息区域（垂直布局）
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(5);

    // 卡片标题
    QLabel *titleLabel = new QLabel("安检信息", m_securityCard);
    titleLabel->setObjectName("titleLabel");
    infoLayout->addWidget(titleLabel);

    // 旅客姓名：标签 + 动态文本
    QHBoxLayout *nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("旅客姓名:", m_securityCard));
    m_lblPassengerName = new QLabel(m_securityCard);
    nameLayout->addWidget(m_lblPassengerName);
    nameLayout->addStretch();
    infoLayout->addLayout(nameLayout);

    // 航班号
    QHBoxLayout *flightLayout = new QHBoxLayout();
    flightLayout->addWidget(new QLabel("航班号:", m_securityCard));
    m_lblFlightNo = new QLabel(m_securityCard);
    flightLayout->addWidget(m_lblFlightNo);
    flightLayout->addStretch();
    infoLayout->addLayout(flightLayout);

    // 座位号
    QHBoxLayout *seatLayout = new QHBoxLayout();
    seatLayout->addWidget(new QLabel("座位号:", m_securityCard));
    m_lblSeatNo = new QLabel(m_securityCard);
    seatLayout->addWidget(m_lblSeatNo);
    seatLayout->addStretch();
    infoLayout->addLayout(seatLayout);

    // 目的地
    QHBoxLayout *destLayout = new QHBoxLayout();
    destLayout->addWidget(new QLabel("目的地:", m_securityCard));
    m_lblDestination = new QLabel(m_securityCard);
    destLayout->addWidget(m_lblDestination);
    destLayout->addStretch();
    infoLayout->addLayout(destLayout);

    // 航班日期
    QHBoxLayout *dateLayout = new QHBoxLayout();
    dateLayout->addWidget(new QLabel("航班日期:", m_securityCard));
    m_lblFlightDate = new QLabel(m_securityCard);
    dateLayout->addWidget(m_lblFlightDate);
    dateLayout->addStretch();
    infoLayout->addLayout(dateLayout);

    // 安检通道
    QHBoxLayout *channelLayout = new QHBoxLayout();
    channelLayout->addWidget(new QLabel("安检通道:", m_securityCard));
    m_lblCheckChannel = new QLabel(m_securityCard);
    channelLayout->addWidget(m_lblCheckChannel);
    channelLayout->addStretch();
    infoLayout->addLayout(channelLayout);

    // 安检时间
    QHBoxLayout *timeLayout = new QHBoxLayout();
    timeLayout->addWidget(new QLabel("安检时间:", m_securityCard));
    m_lblCheckTime = new QLabel(m_securityCard);
    timeLayout->addWidget(m_lblCheckTime);
    timeLayout->addStretch();
    infoLayout->addLayout(timeLayout);

    // 拉伸项：信息置顶
    infoLayout->addStretch();
    mainLayout->addLayout(infoLayout);

    // 右侧人像图区域
    QVBoxLayout *imageLayout = new QVBoxLayout();
    imageLayout->setAlignment(Qt::AlignCenter);
    imageLayout->addWidget(new QLabel("人像图", m_securityCard));

    m_lblPortrait = new QLabel(m_securityCard);
    m_lblPortrait->setFixedSize(120, 160);
    m_lblPortrait->setAlignment(Qt::AlignCenter);
    m_lblPortrait->setStyleSheet("border: 1px solid #cccccc; background-color: #f5f5f5;");
    imageLayout->addWidget(m_lblPortrait);

    mainLayout->addLayout(imageLayout);
}

// ===================== 质控信息卡片初始化 =====================
void PersonBaggageViewWidget::initQualityControlCard()
{
    m_qualityCard = new QFrame(this);

    QHBoxLayout *mainLayout = new QHBoxLayout(m_qualityCard);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(20);

    // 左侧信息区域（垂直布局）
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(5);

    // 卡片标题
    QLabel *titleLabel = new QLabel("质控信息", m_qualityCard);
    titleLabel->setObjectName("titleLabel");
    infoLayout->addWidget(titleLabel);

    // 通道
    QHBoxLayout *channelLayout = new QHBoxLayout();
    channelLayout->addWidget(new QLabel("通  道:", m_qualityCard));
    m_lblQcChannel = new QLabel(m_qualityCard);
    channelLayout->addWidget(m_lblQcChannel);
    channelLayout->addStretch();
    infoLayout->addLayout(channelLayout);

    // 图像来源
    QHBoxLayout *sourceLayout = new QHBoxLayout();
    sourceLayout->addWidget(new QLabel("图像来源:", m_qualityCard));
    m_lblImageSource = new QLabel(m_qualityCard);
    sourceLayout->addWidget(m_lblImageSource);
    sourceLayout->addStretch();
    infoLayout->addLayout(sourceLayout);

    // 判图时间
    QHBoxLayout *judgeTimeLayout = new QHBoxLayout();
    judgeTimeLayout->addWidget(new QLabel("判图时间:", m_qualityCard));
    m_lblJudgeTime = new QLabel(m_qualityCard);
    judgeTimeLayout->addWidget(m_lblJudgeTime);
    judgeTimeLayout->addStretch();
    infoLayout->addLayout(judgeTimeLayout);

    // 判图结果
    QHBoxLayout *judgeResultLayout = new QHBoxLayout();
    judgeResultLayout->addWidget(new QLabel("判图结果:", m_qualityCard));
    m_lblJudgeResult = new QLabel(m_qualityCard);
    judgeResultLayout->addWidget(m_lblJudgeResult);
    judgeResultLayout->addStretch();
    infoLayout->addLayout(judgeResultLayout);

    // AI结果
    QHBoxLayout *aiResultLayout = new QHBoxLayout();
    aiResultLayout->addWidget(new QLabel("AI 结果:", m_qualityCard));
    m_lblAiResult = new QLabel(m_qualityCard);
    aiResultLayout->addWidget(m_lblAiResult);
    aiResultLayout->addStretch();
    infoLayout->addLayout(aiResultLayout);

    // 质控时间
    QHBoxLayout *qcTimeLayout = new QHBoxLayout();
    qcTimeLayout->addWidget(new QLabel("质控时间:", m_qualityCard));
    m_lblQcTime = new QLabel(m_qualityCard);
    qcTimeLayout->addWidget(m_lblQcTime);
    qcTimeLayout->addStretch();
    infoLayout->addLayout(qcTimeLayout);

    // 质控结果
    QHBoxLayout *qcResultLayout = new QHBoxLayout();
    qcResultLayout->addWidget(new QLabel("质控结果:", m_qualityCard));
    m_lblQcResult = new QLabel(m_qualityCard);
    qcResultLayout->addWidget(m_lblQcResult);
    qcResultLayout->addStretch();
    infoLayout->addLayout(qcResultLayout);

    // 拉伸项：信息置顶
    infoLayout->addStretch();
    mainLayout->addLayout(infoLayout);

    // 右侧X光图区域
    QVBoxLayout *imageLayout = new QVBoxLayout();
    imageLayout->setAlignment(Qt::AlignCenter);
    imageLayout->addWidget(new QLabel("质控X光图", m_qualityCard));

    m_lblQcXray = new QLabel(m_qualityCard);
    m_lblQcXray->setFixedSize(280, 220);
    m_lblQcXray->setAlignment(Qt::AlignCenter);
    m_lblQcXray->setStyleSheet("border: 1px solid #cccccc; background-color: #f5f5f5;");
    imageLayout->addWidget(m_lblQcXray);

    mainLayout->addLayout(imageLayout);
}

// ===================== 人包对应信息卡片初始化 =====================
void PersonBaggageViewWidget::initPersonBaggageCard()
{
    m_baggageCard = new QFrame(this);

    QHBoxLayout *mainLayout = new QHBoxLayout(m_baggageCard);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(20);

    // 左侧：旅客信息 + 分页
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(10);

    // 分页控制栏
    QHBoxLayout *pageLayout = new QHBoxLayout();
    pageLayout->setSpacing(10);

    // 卡片标题
    QLabel *titleLabel = new QLabel("人包对应信息", m_baggageCard);
    titleLabel->setObjectName("titleLabel");
    pageLayout->addWidget(titleLabel);

    // 拉伸项：分页按钮右对齐
    pageLayout->addStretch();

    // 上一页按钮
    QPushButton *prevBtn = new QPushButton("←", m_baggageCard);
    prevBtn->setFixedSize(25, 25);
    connect(prevBtn, &QPushButton::clicked, this, [this]() {
        if (m_currentPage > 1) {
            m_currentPage--;
            m_lblPageInfo->setText(QString("%1/%2").arg(m_currentPage).arg(m_totalPage));
        }
    });
    pageLayout->addWidget(prevBtn);

    // 页码显示
    m_lblPageInfo = new QLabel(QString("%1/%2").arg(m_currentPage).arg(m_totalPage), m_baggageCard);
    pageLayout->addWidget(m_lblPageInfo);

    // 下一页按钮
    QPushButton *nextBtn = new QPushButton("→", m_baggageCard);
    nextBtn->setFixedSize(25, 25);
    connect(nextBtn, &QPushButton::clicked, this, [this]() {
        if (m_currentPage < m_totalPage) {
            m_currentPage++;
            m_lblPageInfo->setText(QString("%1/%2").arg(m_currentPage).arg(m_totalPage));
        }
    });
    pageLayout->addWidget(nextBtn);

    leftLayout->addLayout(pageLayout);

    // 旅客头像
    m_lblBaggagePortrait = new QLabel(m_baggageCard);
    m_lblBaggagePortrait->setFixedSize(120, 160);
    m_lblBaggagePortrait->setAlignment(Qt::AlignCenter);
    m_lblBaggagePortrait->setStyleSheet("border: 1px solid #cccccc; background-color: #f5f5f5;");
    leftLayout->addWidget(m_lblBaggagePortrait, 0, Qt::AlignCenter);

    // 拉伸项：内容置顶
    leftLayout->addStretch();
    mainLayout->addLayout(leftLayout);

    // 中间：行李外观图
    QVBoxLayout *middleLayout = new QVBoxLayout();
    middleLayout->setAlignment(Qt::AlignCenter);
    middleLayout->addWidget(new QLabel("行李外观", m_baggageCard));

    m_lblBagAppearance = new QLabel(m_baggageCard);
    m_lblBagAppearance->setFixedSize(280, 220);
    m_lblBagAppearance->setAlignment(Qt::AlignCenter);
    m_lblBagAppearance->setStyleSheet("border: 1px solid #cccccc; background-color: #f5f5f5;");
    middleLayout->addWidget(m_lblBagAppearance);

    mainLayout->addLayout(middleLayout);

    // 右侧：过检X光图
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(new QLabel("过检X光图", m_baggageCard));

    m_lblBagXray = new QLabel(m_baggageCard);
    m_lblBagXray->setFixedSize(280, 220);
    m_lblBagXray->setAlignment(Qt::AlignCenter);
    m_lblBagXray->setStyleSheet("border: 1px solid #cccccc; background-color: #f5f5f5;");
    rightLayout->addWidget(m_lblBagXray);

    mainLayout->addLayout(rightLayout);

    // 摄像头按钮
    QPushButton *cameraBtn = new QPushButton("📷", m_baggageCard);
    cameraBtn->setObjectName("cameraBtn");
    cameraBtn->setFixedSize(30, 30);
    mainLayout->addWidget(cameraBtn, 0, Qt::AlignTop);
}

// ===================== 核心数据更新函数 =====================
void PersonBaggageViewWidget::updateData(const PersonBaggageData &data)
{
    // 1. 更新安检信息
    m_lblPassengerName->setText(data.securityData.passengerName);
    m_lblFlightNo->setText(data.securityData.flightNo);
    m_lblSeatNo->setText(data.securityData.seatNo);
    m_lblDestination->setText(data.securityData.destination);
    m_lblFlightDate->setText(data.securityData.flightDate);
    m_lblCheckChannel->setText(data.securityData.checkChannel);
    m_lblCheckTime->setText(data.securityData.checkTime);

    if (data.securityData.portraitPix.isNull()) {
        m_lblPortrait->setText("暂无图片");
    } else {
        m_lblPortrait->setPixmap(data.securityData.portraitPix.scaled(
            120, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // 2. 更新质控信息
    m_lblQcChannel->setText(data.qualityData.qcChannel);
    m_lblImageSource->setText(data.qualityData.imageSource);
    m_lblJudgeTime->setText(data.qualityData.judgeTime);
    m_lblJudgeResult->setText(data.qualityData.judgeResult);
    m_lblAiResult->setText(data.qualityData.aiResult);
    m_lblQcTime->setText(data.qualityData.qcTime);
    m_lblQcResult->setText(data.qualityData.qcResult);

    // 判图结果颜色控制
    m_lblJudgeResult->setObjectName(data.qualityData.isJudgePass ? "pass" : "");
    m_lblJudgeResult->style()->unpolish(m_lblJudgeResult);
    m_lblJudgeResult->style()->polish(m_lblJudgeResult);

    // AI结果颜色控制
    m_lblAiResult->setObjectName(data.qualityData.isAiSuspicious ? "suspicious" : "");
    m_lblAiResult->style()->unpolish(m_lblAiResult);
    m_lblAiResult->style()->polish(m_lblAiResult);

    // 质控X光图
    if (data.qualityData.qcXrayPix.isNull()) {
        m_lblQcXray->setText("暂无图片");
    } else {
        m_lblQcXray->setPixmap(data.qualityData.qcXrayPix.scaled(
            280, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // 3. 更新人包对应信息
    if (data.baggageData.baggagePortraitPix.isNull()) {
        m_lblBaggagePortrait->setText("暂无图片");
    } else {
        m_lblBaggagePortrait->setPixmap(data.baggageData.baggagePortraitPix.scaled(
            120, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    if (data.baggageData.bagAppearancePix.isNull()) {
        m_lblBagAppearance->setText("暂无图片");
    } else {
        m_lblBagAppearance->setPixmap(data.baggageData.bagAppearancePix.scaled(
            280, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    if (data.baggageData.bagXrayPix.isNull()) {
        m_lblBagXray->setText("暂无图片");
    } else {
        m_lblBagXray->setPixmap(data.baggageData.bagXrayPix.scaled(
            280, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // 更新页码
    m_currentPage = data.baggageData.currentPage;
    m_totalPage = data.baggageData.totalPage;
    m_lblPageInfo->setText(QString("%1/%2").arg(m_currentPage).arg(m_totalPage));
}

// ===================== 单独更新子结构体的接口 =====================
void PersonBaggageViewWidget::updateSecurityData(const SecurityCheckData &data)
{
    m_lblPassengerName->setText(data.passengerName);
    m_lblFlightNo->setText(data.flightNo);
    m_lblSeatNo->setText(data.seatNo);
    m_lblDestination->setText(data.destination);
    m_lblFlightDate->setText(data.flightDate);
    m_lblCheckChannel->setText(data.checkChannel);
    m_lblCheckTime->setText(data.checkTime);

    if (data.portraitPix.isNull()) {
        m_lblPortrait->setText("暂无图片");
    } else {
        m_lblPortrait->setPixmap(data.portraitPix.scaled(
            120, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void PersonBaggageViewWidget::updateQualityData(const QualityControlData &data)
{
    m_lblQcChannel->setText(data.qcChannel);
    m_lblImageSource->setText(data.imageSource);
    m_lblJudgeTime->setText(data.judgeTime);
    m_lblJudgeResult->setText(data.judgeResult);
    m_lblAiResult->setText(data.aiResult);
    m_lblQcTime->setText(data.qcTime);
    m_lblQcResult->setText(data.qcResult);

    m_lblJudgeResult->setObjectName(data.isJudgePass ? "pass" : "");
    m_lblJudgeResult->style()->unpolish(m_lblJudgeResult);
    m_lblJudgeResult->style()->polish(m_lblJudgeResult);

    m_lblAiResult->setObjectName(data.isAiSuspicious ? "suspicious" : "");
    m_lblAiResult->style()->unpolish(m_lblAiResult);
    m_lblAiResult->style()->polish(m_lblAiResult);

    if (data.qcXrayPix.isNull()) {
        m_lblQcXray->setText("暂无图片");
    } else {
        m_lblQcXray->setPixmap(data.qcXrayPix.scaled(
            280, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void PersonBaggageViewWidget::updateBaggageData(const PersonBaggageRelData &data)
{
    if (data.baggagePortraitPix.isNull()) {
        m_lblBaggagePortrait->setText("暂无图片");
    } else {
        m_lblBaggagePortrait->setPixmap(data.baggagePortraitPix.scaled(
            120, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    if (data.bagAppearancePix.isNull()) {
        m_lblBagAppearance->setText("暂无图片");
    } else {
        m_lblBagAppearance->setPixmap(data.bagAppearancePix.scaled(
            280, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    if (data.bagXrayPix.isNull()) {
        m_lblBagXray->setText("暂无图片");
    } else {
        m_lblBagXray->setPixmap(data.bagXrayPix.scaled(
            280, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    m_currentPage = data.currentPage;
    m_totalPage = data.totalPage;
    m_lblPageInfo->setText(QString("%1/%2").arg(m_currentPage).arg(m_totalPage));
}

// ===================== 支持样式表的paintEvent =====================
void PersonBaggageViewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}