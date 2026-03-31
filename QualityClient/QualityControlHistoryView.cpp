#include "QualityControlHistoryView.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSet>

#include "services/qualitycontrolhistoryservice.h"
#include "widgets/LayerViewWidget.h"
#include "logging/logcategories.h"

namespace {
QString firstNonEmpty(const QString &a, const QString &b, const QString &fallback = QStringLiteral("--"))
{
    if (!a.trimmed().isEmpty()) {
        return a;
    }
    if (!b.trimmed().isEmpty()) {
        return b;
    }
    return fallback;
}

}

QualityControlHistoryView::QualityControlHistoryView(QWidget *parent)
    : QualityControlBaseView(parent)
    , m_backButton(nullptr)
    , m_titleLabel(nullptr)
    , m_service(new QualityControlHistoryService(this))
{
    initializeLayout();
    qCInfo(lcQcView) << "[HistoryView] initialized";

    connect(this, &QualityControlBaseView::startCheckRequested, m_service, &QualityControlHistoryService::startCheck);
    connect(this, &QualityControlBaseView::passRequested, m_service, &QualityControlHistoryService::pass);
    connect(this, &QualityControlBaseView::detailRequested, m_service, &QualityControlHistoryService::openPersonDetail);
    connect(this, &QualityControlBaseView::detailRequested, this, &QualityControlHistoryView::requestPersonBaggagePage);
    connect(this, &QualityControlBaseView::xrayTypeToggled, m_service, &QualityControlHistoryService::switchXrayType);
}

void QualityControlHistoryView::setImageDistributeInfo(const ImageDistributeInfo &info)
{
    qCInfo(lcQcView) << "[HistoryView] setImageDistributeInfo taskId=" << info.id << ", type=" << info.type;
    m_currentDistributeInfo = info;
    setBottomBarTaskActive(true);
    setBrandByType(info.type);

    setChannelText(firstNonEmpty(info.channelNoValue, info.channelNo));
    setTimeText(firstNonEmpty(info.qualityControlTime, info.checkTime));
    setFreshnessText(info.freshness.trimmed().isEmpty() ? QStringLiteral("--") : info.freshness);

    if (info.qualityResult.isNull()) {
        setJudgeResultText(QStringLiteral("--"));
    } else {
        setJudgeResultText(info.qualityResult.toString());
    }

    configureTaskImages(info);
}

void QualityControlHistoryView::clearImageDistributeInfo()
{
    qCInfo(lcQcView) << "[HistoryView] clearImageDistributeInfo";
    m_currentDistributeInfo = ImageDistributeInfo();
    clearBaseViewState();
}

QWidget *QualityControlHistoryView::buildTopBar()
{
    QWidget *topBar = new QWidget(contentParent());
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
    connect(m_backButton, &QPushButton::clicked, this, [this]() {
        qCInfo(lcQcView) << "[HistoryView] back clicked";
        emit requestBack();
    });

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
