#ifndef DATETIMEPICKER_H
#define DATETIMEPICKER_H

#include <QWidget>
#include <QDateTime>

class QCalendarWidget;
class QFrame;
class QLineEdit;
class QListWidget;
class QToolButton;

class DateTimePicker : public QWidget
{
    Q_OBJECT
public:
    explicit DateTimePicker(QWidget *parent = nullptr);

    void setDateTime(const QDateTime &dt);
    QDateTime dateTime() const;

signals:
    void dateTimeChanged(const QDateTime &dt);

private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void initPopupUi();
    void updateDisplayText();
    void updatePopupSelection();
    void showPopup();
    void handleDateChanged(const QDate &date);
    void handleTimeChanged();

    QDateTime m_currentDateTime;
    QLineEdit *m_display;
    QToolButton *m_dropButton;
    QFrame *m_popup;
    QCalendarWidget *m_calendar;
    QListWidget *m_hourList;
    QListWidget *m_minList;
    QListWidget *m_secList;
    bool m_suppressTimeSync;
};

#endif // DATETIMEPICKER_H
