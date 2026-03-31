#include "VideoPlaybackView.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLoggingCategory>

#include "logging/logcategories.h"
#include "services/hikvisionsdkservice.h"
#include "services/settingsservice.h"
#include "ui_VideoPlaybackView.h"
#include "widgets/TimeLineSlider.h"

VideoPlaybackView::VideoPlaybackView(QWidget *parent)
    : QWidget(parent), ui(new Ui::VideoPlaybackView) {
  ui->setupUi(this);
  initTimeLineSlider();
  initSpeedCombo();
  startPlaybackTimer();

  if (ui->m_playBtn) {
    connect(ui->m_playBtn, &QPushButton::clicked, this,
            [this]() { handlePlayClicked(); });
  }
  if (ui->backButton) {
    connect(ui->backButton, &QPushButton::clicked, this,
            [this]() { handleBackClicked(); });
  }
  if (ui->m_prevBtn) {
    connect(ui->m_prevBtn, &QPushButton::clicked, this,
            [this]() { handlePrevClicked(); });
  }
  if (ui->m_nextBtn) {
    connect(ui->m_nextBtn, &QPushButton::clicked, this,
            [this]() { handleNextClicked(); });
  }

  m_shiftMinutes = SettingsService::instance().videoBackTimeSpan();

  m_seekTimer = new QTimer(this);
  m_seekTimer->setSingleShot(true);
  m_seekTimer->setInterval(150);
  connect(m_seekTimer, &QTimer::timeout, this,
          [this]() { scheduleSeekForSlider(m_pendingSeekValue); });
}

VideoPlaybackView::~VideoPlaybackView() {
  stopPlaybackTimer();
  stopDualPlayback();
  delete ui;
}

void VideoPlaybackView::configurePlaybackRange(const QDateTime &start,
                                               const QDateTime &end) {
  if (!start.isValid() || !end.isValid() || start >= end) {
    return;
  }

  m_startTime = start;
  m_endTime = end;

  updateTimeLabels();

  if (m_timeLine) {
    m_timeLine->setValue(m_timeLine->minimum());
  }
}

void VideoPlaybackView::setTimeLineNodes(const QList<TimeLineNode> &nodes) {
  if (!m_timeLine) {
    return;
  }
  m_timeLine->setNodes(nodes);
}

bool VideoPlaybackView::startDualPlayback(const QString &ip, int port,
                                          const QString &user,
                                          const QString &password,
                                          int frontChannel, int backChannel,
                                          const QDateTime &start,
                                          const QDateTime &end) {
  if (!ui->m_frontVideo || !ui->m_backVideo) {
    qCWarning(lcPlaybackView) << "Playback widgets are null";
    return false;
  }

  configurePlaybackRange(start, end);

  HikvisionSdkService &sdk = HikvisionSdkService::instance();
  if (!sdk.initialize()) {
    qCWarning(lcPlaybackView) << "SDK init failed";
    return false;
  }
  if (!sdk.login(ip, port, user, password)) {
    qCWarning(lcPlaybackView) << "SDK login failed";
    return false;
  }

  const bool okFront = sdk.startPlaybackByTime(HikvisionSdkService::FrontSlot,
                                               ui->m_frontVideo->winId(),
                                               frontChannel, start, end);
  const bool okBack = sdk.startPlaybackByTime(HikvisionSdkService::BackSlot,
                                              ui->m_backVideo->winId(),
                                              backChannel, start, end);
  if (!okFront || !okBack) {
    qCWarning(lcPlaybackView) << "Start dual playback failed"
                              << "front=" << okFront << "back=" << okBack;
    if (okFront) {
      sdk.stopPlayback(HikvisionSdkService::FrontSlot);
    }
    if (okBack) {
      sdk.stopPlayback(HikvisionSdkService::BackSlot);
    }
    return false;
  }
  m_paused = false;
  sdk.setPlaybackSpeedAll(m_currentSpeed);
  if (ui->m_speedCombo) {
    for (int i = 0; i < ui->m_speedCombo->count(); ++i) {
      bool ok = false;
      const double v = ui->m_speedCombo->itemData(i).toDouble(&ok);
      if (ok && qFuzzyCompare(v, m_currentSpeed)) {
        ui->m_speedCombo->setCurrentIndex(i);
        break;
      }
    }
  }
  qCInfo(lcPlaybackView) << "Dual playback started";
  m_lastIp = ip;
  m_lastPort = port;
  m_lastUser = user;
  m_lastPassword = password;
  m_lastFrontChannel = frontChannel;
  m_lastBackChannel = backChannel;
  m_hasSessionParams = true;
  return true;
}

void VideoPlaybackView::stopDualPlayback() {
  HikvisionSdkService::instance().stopPlaybackAll();
  m_paused = true;
  qCInfo(lcPlaybackView) << "Dual playback stopped";
}

void VideoPlaybackView::initTimeLineSlider() {
  if (!ui->m_timeLineSlider) {
    return;
  }

  auto *layout = new QHBoxLayout(ui->m_timeLineSlider);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  m_timeLine = new TimeLineSlider(ui->m_timeLineSlider);
  m_timeLine->setBubbleEnabled(true);
  m_timeLine->setSnapToNode(false);
  m_timeLine->setRange(0, 10000);

  layout->addWidget(m_timeLine);

  connect(m_timeLine, &QAbstractSlider::sliderMoved, this,
          [this](int value) { handleSliderMoved(value); });
  connect(m_timeLine, &QAbstractSlider::sliderPressed, this,
          [this]() { m_userDragging = true; });
  connect(m_timeLine, &QAbstractSlider::sliderReleased, this, [this]() {
    m_userDragging = false;
    if (m_seekTimer) {
      m_seekTimer->start();
    }
  });
}

void VideoPlaybackView::handleSliderMoved(int value) {
  if (!m_startTime.isValid() || !m_endTime.isValid()) {
    return;
  }
  if (!m_timeLine) {
    return;
  }

  m_pendingSeekValue = value;
  if (m_seekTimer) {
    m_seekTimer->start();
  }
}

QDateTime VideoPlaybackView::timeFromSliderValue(int value) const {
  if (!m_timeLine || !m_startTime.isValid() || !m_endTime.isValid()) {
    return QDateTime();
  }

  const int minV = m_timeLine->minimum();
  const int maxV = m_timeLine->maximum();
  const int span = qMax(1, maxV - minV);
  const double ratio =
      static_cast<double>(value - minV) / static_cast<double>(span);
  const qint64 msec = m_startTime.msecsTo(m_endTime);
  return m_startTime.addMSecs(static_cast<qint64>(msec * ratio));
}

void VideoPlaybackView::updateTimeLabels() {
  if (ui->m_startDateLabel) {
    ui->m_startDateLabel->setText(m_startTime.date().toString("yyyy-MM-dd"));
  }
  if (ui->m_startTimeLabel) {
    ui->m_startTimeLabel->setText(m_startTime.time().toString("HH:mm:ss"));
  }
  if (ui->m_endTimeLabel) {
    ui->m_endTimeLabel->setText(m_endTime.time().toString("HH:mm:ss"));
  }
}

void VideoPlaybackView::startPlaybackTimer() {
  if (m_playbackTimer) {
    return;
  }
  m_playbackTimer = new QTimer(this);
  m_playbackTimer->setInterval(500);
  connect(m_playbackTimer, &QTimer::timeout, this,
          [this]() { syncSliderFromPlayback(); });
  m_playbackTimer->start();
}

void VideoPlaybackView::stopPlaybackTimer() {
  if (!m_playbackTimer) {
    return;
  }
  m_playbackTimer->stop();
}

void VideoPlaybackView::syncSliderFromPlayback() {
  if (!m_timeLine || m_userDragging) {
    return;
  }
  if (!m_startTime.isValid() || !m_endTime.isValid()) {
    return;
  }

  HikvisionSdkService &sdk = HikvisionSdkService::instance();
  QDateTime frontTime;
  QDateTime backTime;
  bool frontOk = sdk.playbackOsdTime(HikvisionSdkService::FrontSlot, frontTime);
  bool backOk = sdk.playbackOsdTime(HikvisionSdkService::BackSlot, backTime);

  auto validInRange = [this](const QDateTime &t) {
    return t.isValid() && t >= m_startTime && t <= m_endTime;
  };
  if (!validInRange(frontTime)) {
    frontOk = false;
  }
  if (!validInRange(backTime)) {
    backOk = false;
  }

  QDateTime chosenTime;
  if (frontOk && backOk) {
    const qint64 delta = qAbs(frontTime.msecsTo(backTime));
    // 误差很小就取平均，偏差大就取较小者避免领先
    if (delta <= 2000) {
      const qint64 mid = frontTime.toMSecsSinceEpoch() + delta / 2;
      chosenTime = QDateTime::fromMSecsSinceEpoch(mid);
    } else {
      chosenTime = (frontTime <= backTime) ? frontTime : backTime;
    }
  } else if (frontOk) {
    chosenTime = frontTime;
  } else if (backOk) {
    chosenTime = backTime;
  } else {
    return;
  }

  if (m_lastOsdTime.isValid()) {
    // 防止异常时间导致回退闪动
    if (chosenTime < m_lastOsdTime.addSecs(-2)) {
      chosenTime = m_lastOsdTime;
    }
  }
  m_lastOsdTime = chosenTime;

  const int value = sliderValueFromTime(chosenTime);
  if (value >= m_timeLine->minimum() && value <= m_timeLine->maximum()) {
    m_timeLine->setValue(value);
  }
}

int VideoPlaybackView::sliderValueFromTime(const QDateTime &time) const {
  if (m_timeLine == nullptr || !time.isValid() || !m_startTime.isValid() ||
      !m_endTime.isValid()) {
    return m_timeLine ? m_timeLine->minimum() : 0;
  }

  const qint64 span = qMax<qint64>(1, m_startTime.msecsTo(m_endTime));
  const qint64 offset = qBound<qint64>(0, m_startTime.msecsTo(time), span);
  const double ratio = static_cast<double>(offset) / static_cast<double>(span);

  const int minV = m_timeLine->minimum();
  const int maxV = m_timeLine->maximum();
  return static_cast<int>(minV + ratio * (maxV - minV));
}

void VideoPlaybackView::scheduleSeekForSlider(int value) {
  if (!m_timeLine || !m_startTime.isValid() || !m_endTime.isValid()) {
    return;
  }
  const QDateTime target = timeFromSliderValue(value);
  HikvisionSdkService::instance().seekPlaybackTimeAll(target);
}

void VideoPlaybackView::handlePlayClicked() {
  HikvisionSdkService &sdk = HikvisionSdkService::instance();
  const bool targetPause = !m_paused;
  if (sdk.pausePlaybackAll(targetPause)) {
    m_paused = targetPause;
    qCInfo(lcPlaybackView) << "Playback pause toggled" << m_paused;
  }
}

void VideoPlaybackView::handleBackClicked() {
  stopPlaybackTimer();
  stopDualPlayback();
  emit backRequested();
}

void VideoPlaybackView::handlePrevClicked() {
  if (!shiftPlaybackRange(-m_shiftMinutes)) {
    qCWarning(lcPlaybackView) << "Shift to previous range failed";
  }
}

void VideoPlaybackView::handleNextClicked() {
  if (!shiftPlaybackRange(m_shiftMinutes)) {
    qCWarning(lcPlaybackView) << "Shift to next range failed";
  }
}

void VideoPlaybackView::initSpeedCombo() {
  if (!ui->m_speedCombo) {
    return;
  }
  if (ui->m_speedCombo->count() == 0) {
    ui->m_speedCombo->addItem("4x", 4.0);
    ui->m_speedCombo->addItem("2x", 2.0);
    ui->m_speedCombo->addItem("1x", 1.0);
    ui->m_speedCombo->addItem("0.5x", 0.5);
    ui->m_speedCombo->addItem("0.25x", 0.25);
  }
  ui->m_speedCombo->setCurrentIndex(2);
  connect(ui->m_speedCombo, qOverload<int>(&QComboBox::currentIndexChanged),
          this, [this](int index) { handleSpeedChanged(index); });
}

void VideoPlaybackView::handleSpeedChanged(int index) {
  if (!ui->m_speedCombo) {
    return;
  }
  if (index < 0 || index >= ui->m_speedCombo->count()) {
    return;
  }
  const QVariant data = ui->m_speedCombo->itemData(index);
  bool ok = false;
  const double speed = data.toDouble(&ok);
  if (!ok) {
    return;
  }
  if (speed <= 0.0) {
    return;
  }
  if (qFuzzyCompare(speed, m_currentSpeed)) {
    return;
  }
  static const double allowedSpeeds[] = {4.0, 2.0, 1.0, 0.5, 0.25};
  bool allowed = false;
  for (double v : allowedSpeeds) {
    if (qFuzzyCompare(speed, v)) {
      allowed = true;
      break;
    }
  }
  if (!allowed) {
    return;
  }

  HikvisionSdkService &sdk = HikvisionSdkService::instance();
  if (!sdk.hasAnyPlayback()) {
    m_currentSpeed = speed;
    return;
  }
  if (sdk.setPlaybackSpeedAll(speed)) {
    m_currentSpeed = speed;
    qCInfo(lcPlaybackView) << "Speed changed to" << speed;
  }
}

bool VideoPlaybackView::shiftPlaybackRange(int minutes) {
  if (!m_hasSessionParams) {
    qCWarning(lcPlaybackView) << "No playback session params";
    return false;
  }
  if (!m_startTime.isValid() || !m_endTime.isValid()) {
    qCWarning(lcPlaybackView) << "Invalid playback range";
    return false;
  }
  if (minutes == 0) {
    return true;
  }

  const QDateTime newStart = m_startTime.addSecs(minutes * 60);
  const QDateTime newEnd = m_endTime.addSecs(minutes * 60);
  if (newStart >= newEnd) {
    qCWarning(lcPlaybackView) << "Invalid shifted range";
    return false;
  }

  HikvisionSdkService &sdk = HikvisionSdkService::instance();
  const auto frontFiles = sdk.findFilesByChannel(m_lastFrontChannel, newStart, newEnd, 1, true);
  const auto backFiles = sdk.findFilesByChannel(m_lastBackChannel, newStart, newEnd, 1, true);
  if (frontFiles.isEmpty() && backFiles.isEmpty()) {
    qCWarning(lcPlaybackView) << "No playback files in shifted range";
    return false;
  }

  stopDualPlayback();
  const bool ok = startDualPlayback(m_lastIp, m_lastPort, m_lastUser, m_lastPassword,
                                    m_lastFrontChannel, m_lastBackChannel,
                                    newStart, newEnd);
  if (!ok) {
    qCWarning(lcPlaybackView) << "Restart playback for shifted range failed";
    return false;
  }

  configurePlaybackRange(newStart, newEnd);
  return true;
}
