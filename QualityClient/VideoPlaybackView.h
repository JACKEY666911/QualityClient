#ifndef VIDEOPLAYBACKVIEW_H
#define VIDEOPLAYBACKVIEW_H

#include <QDateTime>
#include <QTimer>
#include <QWidget>

#include "widgets/TimeLineSlider.h"

namespace Ui {
class VideoPlaybackView;
}

class VideoPlaybackView : public QWidget {
  Q_OBJECT

public:
  explicit VideoPlaybackView(QWidget *parent = nullptr);
  ~VideoPlaybackView();

  void configurePlaybackRange(const QDateTime &start, const QDateTime &end);
  void setTimeLineNodes(const QList<TimeLineNode> &nodes);
  bool startDualPlayback(const QString &ip, int port, const QString &user,
                         const QString &password, int frontChannel,
                         int backChannel, const QDateTime &start,
                         const QDateTime &end);
  void stopDualPlayback();

signals:
  void backRequested();

private:
  void initTimeLineSlider();
  void handleSliderMoved(int value);
  QDateTime timeFromSliderValue(int value) const;
  void updateTimeLabels();
  void startPlaybackTimer();
  void stopPlaybackTimer();
  void syncSliderFromPlayback();
  int sliderValueFromTime(const QDateTime &time) const;
  void handlePlayClicked();
  void handleBackClicked();
  void handlePrevClicked();
  void handleNextClicked();
  void scheduleSeekForSlider(int value);
  void initSpeedCombo();
  void handleSpeedChanged(int index);
  bool shiftPlaybackRange(int minutes);

  Ui::VideoPlaybackView *ui;
  TimeLineSlider *m_timeLine = nullptr;
  QDateTime m_startTime;
  QDateTime m_endTime;
  QTimer *m_playbackTimer = nullptr;
  QTimer *m_seekTimer = nullptr;
  int m_pendingSeekValue = 0;
  bool m_userDragging = false;
  QDateTime m_lastOsdTime;
  bool m_paused = true;
  double m_currentSpeed = 1.0;
  int m_shiftMinutes = 0;

  QString m_lastIp;
  int m_lastPort = 0;
  QString m_lastUser;
  QString m_lastPassword;
  int m_lastFrontChannel = -1;
  int m_lastBackChannel = -1;
  bool m_hasSessionParams = false;
};

#endif // VIDEOPLAYBACKVIEW_H
