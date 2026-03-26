#include "TimeCounter.h"

TimeCounter::TimeCounter(QObject *parent)
    : QObject{parent}
    , m_mode(CountUp)
    , m_current(0)
    , m_start(0)
{
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &TimeCounter::onTimeout);
}

void TimeCounter::setMode(Mode mode)
{
    m_mode = mode;
}

void TimeCounter::setStartSeconds(int seconds)
{
    if (seconds < 0) {
        seconds = 0;
    }
    m_start = seconds;
    m_current = seconds;
}

void TimeCounter::start()
{
    if (!m_timer.isActive()) {
        m_timer.start();
    }
}

void TimeCounter::pause()
{
    if (m_timer.isActive()) {
        m_timer.stop();
    }
}

void TimeCounter::reset()
{
    bool wasRunning = m_timer.isActive();
    if (wasRunning) {
        m_timer.stop();
    }
    m_current = m_start;
    emit secondsChanged(m_current);
    if (wasRunning) {
        m_timer.start();
    }
}

int TimeCounter::currentSeconds() const
{
    return m_current;
}

void TimeCounter::onTimeout()
{
    if (m_mode == CountUp) {
        m_current++;
    } else {
        m_current--;
        if (m_current <= 0) {
            m_current = 0;
            pause();
            emit finished();
        }
    }
    emit secondsChanged(m_current);
}