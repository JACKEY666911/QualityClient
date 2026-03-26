#ifndef TIMECOUNTER_H
#define TIMECOUNTER_H

#include <QObject>
#include <QTimer>

class TimeCounter : public QObject
{
    Q_OBJECT
public:
    enum Mode {
        CountUp,   // 正计时（秒数 ++）
        CountDown  // 倒计时（秒数 --）
    };
    Q_ENUM(Mode)
    explicit TimeCounter(QObject *parent = nullptr);
    void setMode(Mode mode);
    void setStartSeconds(int seconds);  // 初始秒数
    void start();
    void pause();
    void reset();

    int currentSeconds() const;         // 获取当前秒数
    Mode mode() const { return m_mode; }
    bool isRunning() const { return m_timer.isActive(); }

private slots:
    void onTimeout();

signals:
    void secondsChanged(int seconds);  // 每秒触发，返回当前秒
    void finished();                   // 倒计时到 0 触发

private:
    QTimer m_timer;
    Mode m_mode;
    int m_current;
    int m_start;
};

#endif // TIMECOUNTER_H
