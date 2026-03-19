#ifndef MODERNDATETIMEPICKER_H
#define MODERNDATETIMEPICKER_H

#include <QWidget>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

// --- 1. 现代感时间滚轮组件 (内部使用) ---
class ModernTimeWheel : public QWidget {
    Q_OBJECT
public:
    explicit ModernTimeWheel(int min, int max, QWidget *parent = nullptr);
    int value() const { return m_val; }
    void setValue(int v);

signals:
    void valueChanged(int v);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    int m_min, m_max, m_val;
};

// --- 2. 弹出式日期时间面板 (内部使用) ---
class DateTimePickerPanel : public QWidget {
    Q_OBJECT
public:
    explicit DateTimePickerPanel(QWidget *parent = nullptr);
    void setDateTime(const QDateTime &dt);

signals:
    void dateTimeChanged(const QDateTime &dt);

private slots:
    void updateCalendar();
    void onConfirm();

private:
    void setupUI();

    QFrame *m_container;
    QDate m_viewDate, m_selectedDate;
    QGridLayout *m_calendarGrid;
    QLabel *m_titleLabel;
    ModernTimeWheel *m_hour, *m_min, *m_sec;
};

// --- 3. 最终组合控件 (对外接口) ---
class ModernDateTimeEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit ModernDateTimeEdit(QWidget *parent = nullptr);
    QDateTime dateTime() const { return m_dt; }
    void setDateTime(const QDateTime &dt);

protected:
    void mousePressEvent(QMouseEvent *e) override;

private:
    QDateTime m_dt;
    DateTimePickerPanel *m_panel;
};

#endif // MODERNDATETIMEPICKER_H
