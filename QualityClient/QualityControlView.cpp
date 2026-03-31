#include "QualityControlView.h"

#include <QDateTime>
#include <QDebug>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSet>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QtGlobal>

#include "HistoryMainViewWidget.h"
#include "Models/XrayImage.h"
#include "QualityControlHistoryView.h"
#include "dialogs/appmessagedialog.h"
#include "dialogs/toastnotification.h"
#include "logging/logcategories.h"
#include "PersonBaggageView.h"
#include "services/qualitycontrolservice.h"
#include "services/settingsservice.h"
#include "widgets/LayerViewWidget.h"
#include "widgets/SwitchButton.h"

namespace {
QString firstNonEmpty(const QString &a, const QString &b,
                      const QString &fallback = QStringLiteral("--")) {
  if (!a.trimmed().isEmpty()) {
    return a;
  }
  if (!b.trimmed().isEmpty()) {
    return b;
  }
  return fallback;
}

} // namespace

QualityControlView::QualityControlView(QWidget *parent)
    : QualityControlBaseView(parent), m_taskCountLabel(nullptr),
      m_durationTitleLabel(nullptr), m_durationDot(nullptr),
      m_durationValueLabel(nullptr), m_userLabel(nullptr), m_userName(nullptr),
      m_pauseToggle(nullptr), m_statsButton(nullptr), m_historyButton(nullptr),
      m_switchModeButton(nullptr), m_navStack(nullptr), m_homePage(nullptr),
      m_historyMainView(nullptr), m_historyView(nullptr),
      m_personBaggageView(nullptr), m_testIndex(0), m_secCounter(nullptr),
      m_elapsedSeconds(0), m_hasCurrentTask(false),
      m_pullState(PullState::Paused), m_consecutiveFetchErrors(0),
      m_service(new QualityControlService(this)) {
  m_navStack = new QStackedWidget;
  QVBoxLayout *rootLayout = new QVBoxLayout;
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->setSpacing(0);
  rootLayout->addWidget(m_navStack);
  setLayout(rootLayout);
  setMinimumSize(1920, 1080);
  setMaximumSize(1920, 1080);

  m_homePage = new QWidget;
  m_navStack->addWidget(m_homePage);
  initializeLayout(m_homePage);
  clearImageDistributeInfo();

  connect(this, &QualityControlBaseView::xrayTypeToggled, m_service,
          &QualityControlService::switchXrayType);
  connect(this, &QualityControlBaseView::startCheckRequested, this,
          &QualityControlView::handleStartCheckClicked);
  connect(this, &QualityControlBaseView::passRequested, this,
          &QualityControlView::handlePassClicked);
  connect(m_service, &QualityControlService::imageDistributeInfoReceived, this,
          &QualityControlView::setImageDistributeInfo);
  connect(m_service, &QualityControlService::errorOccurred, this,
          &QualityControlView::handleServiceError);

  m_historyMainView = new HistoryMainView;
  m_historyView = new QualityControlHistoryView;
  m_personBaggageView = new PersonBaggageView;
  m_navStack->addWidget(m_historyMainView);
  m_navStack->addWidget(m_historyView);
  m_navStack->addWidget(m_personBaggageView);
  // connect(m_historyMainView, &HistoryMainViewWidget::requestBack, this,
  // &QualityControlView::closeHistoryMainView); connect(m_historyMainView,
  // &HistoryMainViewWidget::historyDetailRequested, this,
  // &QualityControlView::openHistoryDetailView);
  connect(m_historyView, &QualityControlHistoryView::requestBack, this,
          &QualityControlView::closeHistoryView);
  connect(m_historyView, &QualityControlHistoryView::requestPersonBaggagePage,
          this, &QualityControlView::openPersonBaggageView);
  connect(m_personBaggageView, &PersonBaggageView::requestBack, this,
          &QualityControlView::closePersonBaggageView);
  connect(this, &QualityControlBaseView::detailRequested, this,
          &QualityControlView::openPersonBaggageView);

  m_navStack->setCurrentWidget(m_homePage);
  updatePullState();
}

void QualityControlView::setUserName(const QString &userName) {
  if (m_userName) {
    m_userName->setText(userName);
  }
  if (m_historyMainView) {
    // m_historyMainView->setUserName(userName);
  }
}

void QualityControlView::setTaskCount(int count) {
  if (m_taskCountLabel) {
    m_taskCountLabel->setText(QStringLiteral("任务数量：%1").arg(count));
  }
}

void QualityControlView::setCountdownText(const QString &text) {
  if (m_durationValueLabel) {
    m_durationValueLabel->setText(text);
  }
}

void QualityControlView::setMainImage(const QPixmap &pixmap) {
  QualityControlBaseView::setMainImage(pixmap);
}

void QualityControlView::setAuxImage(const QPixmap &pixmap) {
  QualityControlBaseView::setAuxImage(pixmap);
}

void QualityControlView::setImageDistributeInfo(
    const ImageDistributeInfo &info) {
  // 收到新任务后先重置子页面与状态，再进入当前任务展示流程。
  qCInfo(lcQcView) << "Receive task id=" << info.id << ", type=" << info.type;
  m_consecutiveFetchErrors = 0;
  resetSubViewsForNewImage();
  m_hasCurrentTask = true;
  setBottomBarTaskActive(true);
  m_currentDistributeInfo = info;
  setBrandByType(info.type);

  setChannelText(firstNonEmpty(info.channelNoValue, info.channelNo));
  setTimeText(firstNonEmpty(info.qualityControlTime, info.checkTime));
  setFreshnessText(info.freshness.trimmed().isEmpty() ? QStringLiteral("--")
                                                      : info.freshness);

  if (info.qualityResult.isNull()) {
    setJudgeResultText(QStringLiteral("--"));
  } else {
    setJudgeResultText(info.qualityResult.toString());
  }

  if (m_taskCountLabel) {
    const int count = info.taskCount.isValid() ? info.taskCount.toInt() : 0;
    m_taskCountLabel->setText(QStringLiteral("任务数量：%1").arg(count));
  }

  // 图片展示统一交给 BaseView：原图优先、Overlay 首屏预取、按钮联动。
  configureTaskImages(info);

  if (m_secCounter) {
    int duration = SettingsService::instance().qcCountdownSec();
    m_secCounter->setStartSeconds(duration);
    handleDurationTick(m_secCounter->currentSeconds());
    if (duration > 0) {
      m_secCounter->start();
    }
  }
  updatePullState();
}

void QualityControlView::clearImageDistributeInfo() {
  // 清空当前任务：UI 复位、计时器归零、等待下一次拉取。
  resetSubViewsForNewImage();
  m_hasCurrentTask = false;
  m_currentDistributeInfo = ImageDistributeInfo();
  clearBaseViewState();
  if (m_taskCountLabel) {
    m_taskCountLabel->setText(QStringLiteral("任务数量：0"));
  }
  if (m_durationValueLabel) {
    m_durationValueLabel->setText(QStringLiteral("00:00:00"));
  }
  if (m_secCounter) {
    m_secCounter->pause();
    m_secCounter->setStartSeconds(0);
  }
}

void QualityControlView::handleStartCheckClicked() {
  qCInfo(lcQcView) << "StartCheck clicked, currentTaskId="
                   << m_currentDistributeInfo.id;
  if (m_service) {
    m_service->startCheck();
  }
  const bool isContinue = m_pauseToggle && m_pauseToggle->isChecked();
  // 提交后立即清屏，避免旧任务残留在界面上。
  clearImageDistributeInfo();
  // 提交后由状态机决定是否继续拉取：
  // - 继续态：拉取下一任务
  // - 暂停态：不拉取
  updatePullState();
  if (isContinue) {
    emit requestNextImageDistributeInfo();
  }
  qCInfo(lcQcView) << "StartCheck post submit continue=" << isContinue
                   << "pullState=" << static_cast<int>(m_pullState);
}

void QualityControlView::handlePassClicked() {
  qCInfo(lcQcView) << "Pass clicked, currentTaskId="
                   << m_currentDistributeInfo.id;
  if (m_service) {
    m_service->pass();
  }
  const bool isContinue = m_pauseToggle && m_pauseToggle->isChecked();
  // 放行后同样清空当前任务，进入等待状态。
  clearImageDistributeInfo();
  // 放行后同样由状态机统一决定拉取行为。
  updatePullState();
  if (isContinue) {
    emit requestNextImageDistributeInfo();
  }
  qCInfo(lcQcView) << "Pass post submit continue=" << isContinue
                   << "pullState=" << static_cast<int>(m_pullState);
}

void QualityControlView::handleServiceError(const QString &message) {
  qCWarning(lcQcView) << "fetchTask error:" << message;
  ++m_consecutiveFetchErrors;
  ToastNotification::showToast(
      this, message.isEmpty() ? QStringLiteral("网络异常，正在重试") : message,
      ToastNotification::Warning, 1000);
  // 连续失败达到阈值后主动熔断，防止界面持续抖动重试。
  if (m_consecutiveFetchErrors == 10) {
    if (m_service) {
      m_service->setPollingEnabled(false);
    }
    if (m_pauseToggle && m_pauseToggle->isChecked()) {
      m_pauseToggle->setSwitchChecked(false);
    }
    clearImageDistributeInfo();
    m_pullState = PullState::Paused;
    m_consecutiveFetchErrors = 0;
    AppMessageDialog::showInfo(
        this, QStringLiteral("网络提示"),
        QStringLiteral("连续多次拉取失败，请检查网络或服务状态。"));
    emit requestModeSwitch();
    return;
  }
  // 出错时保持状态机一致：如果当前无任务且处于继续，则继续等待并轮询。
  updatePullState();
}

void QualityControlView::handlePauseToggled(bool checked) {
  Q_UNUSED(checked)
  // 顶部开关只驱动“拉取状态机”，具体轮询启停由 updatePullState 统一决策。
  updatePullState();
  emit taskPauseToggled(!checked);
}

void QualityControlView::handleDurationTick(int sec) {
  if (m_durationValueLabel) {
    const int hours = sec / 3600;
    const int minutes = (sec % 3600) / 60;
    const int seconds = sec % 60;
    m_durationValueLabel->setText(QStringLiteral("%1:%2:%3")
                                      .arg(hours, 2, 10, QLatin1Char('0'))
                                      .arg(minutes, 2, 10, QLatin1Char('0'))
                                      .arg(seconds, 2, 10, QLatin1Char('0')));
  }
}

void QualityControlView::handleDurationFinish() {
  clearImageDistributeInfo();
  if (m_pauseToggle && m_pauseToggle->isChecked()) {
    m_pauseToggle->setSwitchChecked(false);
  }
  updatePullState();
}

void QualityControlView::openHistoryView() {
  qCInfo(lcQcView) << "[QualityControlView] openHistoryView";
  emit requestHistory();
  QList<ImageDistributeInfo> cards;
  if (!m_currentDistributeInfo.id.trimmed().isEmpty()) {
    cards.append(m_currentDistributeInfo);
  }
  if (m_historyMainView) {
    // m_historyMainView->setHistoryInfos(cards);
  }
  cacheAndPauseForSubView();
  if (!switchToHistoryPage()) {
    restorePauseAfterSubView();
  }
}

void QualityControlView::closeHistoryMainView() {
  if (!m_historyMainView) {
    return;
  }
  qCInfo(lcQcView) << "[QualityControlView] closeHistoryMainView";
  switchToHomePage();
  restorePauseAfterSubView();
}

void QualityControlView::openHistoryDetailView(
    const ImageDistributeInfo &info) {
  qCInfo(lcQcView) << "[QualityControlView] openHistoryDetailView taskId="
                   << info.id;
  if (!switchToHistoryDetailPage(info)) {
    qCWarning(lcQcView)
        << "[QualityControlView] openHistoryDetailView failed: invalid info";
  }
}

void QualityControlView::closeHistoryView() {
  if (!m_historyView) {
    return;
  }
  qCInfo(lcQcView) << "[QualityControlView] closeHistoryView";
  m_historyView->clearImageDistributeInfo();
  switchToHistoryPage();
}

void QualityControlView::openPersonBaggageView() {
  qCInfo(lcQcView) << "[QualityControlView] openPersonBaggageView";
  cacheAndPauseForSubView();
  switchToPersonBaggagePage();
}

void QualityControlView::closePersonBaggageView() {
  if (!m_personBaggageView) {
    return;
  }
  qCInfo(lcQcView) << "[QualityControlView] closePersonBaggageView";
  m_personBaggageView->clearViewData();

  QWidget *returnPage = m_homePage;
  if (!m_subViewSuspendStack.isEmpty() &&
      m_subViewSuspendStack.last().returnPage) {
    returnPage = m_subViewSuspendStack.last().returnPage;
  }
  if (m_navStack && returnPage) {
    m_navStack->setCurrentWidget(returnPage);
  } else {
    switchToHomePage();
  }
  restorePauseAfterSubView();
}

void QualityControlView::cacheAndPauseForSubView() {
  const bool pauseChecked = m_pauseToggle && m_pauseToggle->isChecked();
  const bool counterWasRunning = m_secCounter && m_secCounter->isRunning();
  QWidget *currentPage = m_navStack ? m_navStack->currentWidget() : m_homePage;
  SubViewSuspendState state;
  state.pauseChecked = pauseChecked;
  state.counterWasRunning = counterWasRunning;
  state.returnPage = currentPage ? currentPage : m_homePage;
  m_subViewSuspendStack.append(state);
  qCInfo(lcQcView) << "[QualityControlView] suspend push depth="
                   << m_subViewSuspendStack.size()
                   << "pauseChecked=" << state.pauseChecked
                   << "counterWasRunning=" << state.counterWasRunning;

  if (m_pauseToggle && m_pauseToggle->isChecked()) {
    m_pauseToggle->setSwitchChecked(false);
  }
  if (m_secCounter && m_secCounter->currentSeconds() > 0) {
    m_secCounter->pause();
  }
}

void QualityControlView::restorePauseAfterSubView() {
  if (m_subViewSuspendStack.isEmpty()) {
    return;
  }
  const SubViewSuspendState state = m_subViewSuspendStack.takeLast();
  qCInfo(lcQcView) << "[QualityControlView] suspend pop depth="
                   << m_subViewSuspendStack.size()
                   << "pauseChecked=" << state.pauseChecked
                   << "counterWasRunning=" << state.counterWasRunning;

  if (m_pauseToggle) {
    m_pauseToggle->setSwitchChecked(state.pauseChecked);
  }
  if (!m_secCounter) {
    return;
  }
  if (state.counterWasRunning && m_secCounter->currentSeconds() > 0) {
    m_secCounter->start();
  } else {
    m_secCounter->pause();
  }
}

void QualityControlView::resetSubViewsForNewImage() {
  m_subViewSuspendStack.clear();
  if (m_historyMainView) {
    // m_historyMainView->clearHistoryInfos();
    if (m_navStack && m_navStack->currentWidget() == m_historyMainView) {
      switchToHomePage();
    }
  }
  if (m_historyView) {
    m_historyView->clearImageDistributeInfo();
    if (m_navStack && m_navStack->currentWidget() == m_historyView) {
      switchToHomePage();
    }
  }
  if (m_personBaggageView) {
    m_personBaggageView->clearViewData();
    if (m_navStack && m_navStack->currentWidget() == m_personBaggageView) {
      switchToHomePage();
    }
  }
}

void QualityControlView::switchToHomePage() {
  if (m_navStack && m_homePage) {
    m_navStack->setCurrentWidget(m_homePage);
  }
}

bool QualityControlView::switchToHistoryPage() {
  if (!m_navStack || !m_historyMainView) {
    return false;
  }
  m_navStack->setCurrentWidget(m_historyMainView);
  return true;
}

bool QualityControlView::switchToHistoryDetailPage(
    const ImageDistributeInfo &info) {
  if (info.id.trimmed().isEmpty()) {
    qCWarning(lcQcView) << "[QualityControlView] switchToHistoryDetailPage "
                           "skipped: invalid task";
    return false;
  }
  if (m_navStack && m_historyView) {
    m_navStack->setCurrentWidget(m_historyView);
    m_historyView->setImageDistributeInfo(info);
    return true;
  }
  return false;
}

void QualityControlView::switchToPersonBaggagePage() {
  if (m_navStack && m_personBaggageView) {
    m_navStack->setCurrentWidget(m_personBaggageView);
  }
}

void QualityControlView::updatePullState() {
  // 三态状态机：
  // 1) Paused: 人工暂停
  // 2) WaitingTask: 继续且当前无任务，需要轮询
  // 3) DisplayingTask: 继续但正在展示任务，暂停轮询
  const bool isContinue = m_pauseToggle && m_pauseToggle->isChecked();
  PullState nextState = PullState::Paused;

  if (!isContinue) {
    nextState = PullState::Paused;
  } else if (m_hasCurrentTask) {
    nextState = PullState::DisplayingTask;
  } else {
    nextState = PullState::WaitingTask;
  }

  m_pullState = nextState;
  if (!m_service) {
    return;
  }

  switch (m_pullState) {
  case PullState::Paused:
  case PullState::DisplayingTask:
    m_service->setPollingEnabled(false);
    break;
  case PullState::WaitingTask:
    m_service->setPollingEnabled(true);
    break;
  }
}

QWidget *QualityControlView::buildTopBar() {
  QWidget *topBar = new QWidget(contentParent());
  topBar->setMinimumHeight(55);
  topBar->setMaximumHeight(55);
  topBar->setObjectName(QStringLiteral("qcTopBar"));
  topBar->setStyleSheet(QStringLiteral(
      "QWidget#qcTopBar{background:#117595;}"
      "QLabel{color:white;font-family:\"Microsoft YaHei\";}"
      "QPushButton{background:transparent;color:white;border-radius:4px;"
      "padding:4px 12px;font-family:\"Microsoft YaHei\";}"));

  QHBoxLayout *layout = new QHBoxLayout(topBar);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  QHBoxLayout *layoutLeft = new QHBoxLayout;
  layoutLeft->setContentsMargins(0, 0, 0, 0);
  layoutLeft->setSpacing(10);
  QHBoxLayout *layoutMiddle = new QHBoxLayout;
  QHBoxLayout *layoutRight = new QHBoxLayout;
  layoutRight->setContentsMargins(0, 0, 10, 0);
  layoutRight->setSpacing(10);
  layout->addItem(layoutLeft);
  layout->setStretchFactor(layoutLeft, 2);
  layout->addItem(layoutMiddle);
  layout->setStretchFactor(layoutMiddle, 1);
  layout->addItem(layoutRight);
  layout->setStretchFactor(layoutRight, 2);

  QLabel *logo = new QLabel(topBar);
  QPixmap logoPixmap(QStringLiteral(":/Images/logo.png"));
  logo->setFixedSize(140, 55);
  logo->setPixmap(logoPixmap.scaled(120, 40, Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));
  logo->setAlignment(Qt::AlignCenter);

  QLabel *title = new QLabel(QStringLiteral("般睿安检集中质控系统"), topBar);
  QFont titleFont;
  titleFont.setPointSize(18);
  titleFont.setBold(true);
  title->setFont(titleFont);
  QLabel *taskCountLogo = new QLabel(topBar);
  QPixmap pix(":/Images/envelope.png");
  taskCountLogo->setPixmap(
      pix.scaled(37, 37, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  taskCountLogo->setFixedSize(37, 37);
  m_taskCountLabel = new QLabel(QStringLiteral("任务数量：0"), topBar);
  QFont f;
  f.setBold(true);
  f.setFamily("Microsoft YaHei");
  f.setPixelSize(16);
  m_taskCountLabel->setFont(f);

  m_pauseToggle = new SwitchButton(topBar);
  m_pauseToggle->setChecked(true);
  connect(m_pauseToggle, &QAbstractButton::toggled, this,
          &QualityControlView::handlePauseToggled);

  layoutLeft->addWidget(logo);
  layoutLeft->addWidget(title);
  layoutLeft->addStretch(1);
  layoutLeft->addWidget(taskCountLogo);
  layoutLeft->addWidget(m_taskCountLabel);
  layoutLeft->addStretch(1);
  layoutLeft->addWidget(m_pauseToggle);
  layoutLeft->addStretch(1);

  m_secCounter = new TimeCounter(this);
  m_secCounter->setMode(TimeCounter::CountDown);
  m_secCounter->setStartSeconds(0);
  connect(m_secCounter, &TimeCounter::secondsChanged, this,
          &QualityControlView::handleDurationTick);
  connect(m_secCounter, &TimeCounter::finished, this,
          &QualityControlView::handleDurationFinish);
  handleDurationTick(m_secCounter->currentSeconds());
  m_durationDot = new QLabel(topBar);
  m_durationDot->setFixedSize(32, 32);
  QPixmap m_durationDotPix(":/Images/clock.png");
  m_durationDot->setPixmap(m_durationDotPix.scaled(32, 32, Qt::KeepAspectRatio,
                                                   Qt::SmoothTransformation));

  m_durationTitleLabel = new QLabel(QStringLiteral("时长："), topBar);
  m_durationValueLabel = new QLabel(QStringLiteral("00:00:00"), topBar);
  m_durationTitleLabel->setFont(f);
  m_durationValueLabel->setFont(f);
  layoutMiddle->addWidget(m_durationDot);
  layoutMiddle->addSpacing(5);
  layoutMiddle->addWidget(m_durationTitleLabel);
  layoutMiddle->addWidget(m_durationValueLabel);
  layoutMiddle->setAlignment(Qt::AlignCenter);

  m_statsButton = new QPushButton(QStringLiteral("查看任务统计"), topBar);
  m_statsButton->setFixedSize(150, 35);
  m_statsButton->setFont(f);
  m_statsButton->setStyleSheet("background:#70b603;");
  connect(m_statsButton, &QPushButton::clicked, this,
          &QualityControlView::requestTaskStats);

  m_historyButton = new QPushButton(QStringLiteral("质控记录回查"), topBar);
  m_historyButton->setFixedSize(150, 35);
  m_historyButton->setFont(f);
  m_historyButton->setStyleSheet("background:#01b9ec");
  connect(m_historyButton, &QPushButton::clicked, this,
          &QualityControlView::openHistoryView);

  m_userLabel = new QLabel(topBar);
  m_userLabel->setFixedSize(37, 37);
  m_userLabel->setPixmap(QPixmap(":/Images/user_icon.png").scaled(37, 37));
  m_userLabel->setStyleSheet("background:transparent;");
  m_userName = new QLabel(topBar);
  m_userName->setMinimumSize(30, 34);
  m_userName->setFont(f);

  m_switchModeButton = new QPushButton(topBar);
  m_switchModeButton->setFixedSize(37, 37);
  m_switchModeButton->setIcon(QIcon(":/Images/DisConnected.png"));
  m_switchModeButton->setIconSize(m_switchModeButton->size());
  connect(m_switchModeButton, &QPushButton::clicked, this,
          &QualityControlView::requestModeSwitch);

  layoutRight->addStretch(1);
  layoutRight->addWidget(m_statsButton);
  layoutRight->addSpacing(10);
  layoutRight->addWidget(m_historyButton);
  layoutRight->addSpacing(10);
  layoutRight->addWidget(m_userLabel);
  layoutRight->addWidget(m_userName);
  layoutRight->addWidget(m_switchModeButton);

  return topBar;
}

void QualityControlView::loadNextImage() {
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

void QualityControlView::initTestLayerItems() {
  if (!m_layerView) {
    return;
  }
  QList<XrayImage> items;
  auto makeItem = [](const QString &mainPath, const QString &assistPath,
                     const QString &pbType, qint64 viewCount, bool selected) {
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

  items.append(makeItem(QStringLiteral(":/Images/yisuoTest.png"),
                        QStringLiteral(":/Images/yisuoTest1.png"), QString(), 1,
                        true));
  items.append(makeItem(QStringLiteral(":/Images/yisuoTest1.png"),
                        QStringLiteral(":/Images/yisuoTest2.png"), QString(), 2,
                        false));
  items.append(makeItem(QStringLiteral(":/Images/yisuoTest2.png"),
                        QStringLiteral(":/Images/yisuoTest3.png"),
                        QStringLiteral("801"), 1, false));
  items.append(makeItem(QStringLiteral(":/Images/yisuoTest3.png"),
                        QStringLiteral(":/Images/yisuoTest.png"), QString(), 3,
                        false));
  items.append(makeItem(QStringLiteral(":/Images/yisuoTest.png"),
                        QStringLiteral(":/Images/yisuoTest2.png"), QString(), 1,
                        false));
  items.append(makeItem(QStringLiteral(":/Images/yisuoTest1.png"),
                        QStringLiteral(":/Images/yisuoTest3.png"),
                        QStringLiteral("802"), 1, false));
  items.append(makeItem(QStringLiteral(":/Images/yisuoTest2.png"),
                        QStringLiteral(":/Images/yisuoTest.png"), QString(), 2,
                        false));

  m_layerView->setPageSize(3);
  m_layerView->setXrayItems(items);
  m_layerView->setDurationText(QStringLiteral("10秒"));
}
