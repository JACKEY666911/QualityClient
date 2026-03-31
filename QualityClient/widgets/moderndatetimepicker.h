#ifndef MODERNDATETIMEPICKER_H
#define MODERNDATETIMEPICKER_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QFrame>
#include <QDateTime>
#include <QList>

class ModernTimeWheel : public QWidget {
    Q_OBJECT
public:
    explicit ModernTimeWheel(int min, int max, QWidget *parent = nullptr);
    void setValue(int v);
    int value() const { return m_val; }

signals:
    void valueChanged(int v);

protected:
    void paintEvent(QPaintEvent *) override;
    void wheelEvent(QWheelEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

private:
    int m_min, m_max, m_val;
};

class DateTimePickerPanel : public QWidget {
    Q_OBJECT
public:
    explicit DateTimePickerPanel(QWidget *parent = nullptr);
    void setDateTime(const QDateTime &dt);

signals:
    void dateTimeChanged(const QDateTime &dt);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onConfirm();
    void updateCalendar();

private:
    void setupUI();
    QDate m_selectedDate;
    QDate m_viewDate;
    QLabel *m_titleLabel;
    QGridLayout *m_calendarGrid;
    QList<QPushButton*> m_dateButtons;
    ModernTimeWheel *m_hour, *m_min, *m_sec;
    QFrame *m_container;
};

class ModernDateTimeEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit ModernDateTimeEdit(QWidget *parent = nullptr);
    void setDateTime(const QDateTime &dt);
    QDateTime dateTime() const { return m_dt; }

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QDateTime m_dt;
    DateTimePickerPanel *m_panel;
    qint64 m_lastHideTime = 0;
};

#endif