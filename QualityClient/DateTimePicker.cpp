#include "DateTimePicker.h"

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>

DateTimePicker::DateTimePicker(QWidget *parent)
    : QWidget{parent}
{
    m_currentDateTime = QDateTime::currentDateTime();

    // 主按钮
    m_btn = new QPushButton(this);
    m_btn->setFont(QFont("Microsoft YaHei", 22));
    m_btn->setFixedSize(380, 70);
    updateButtonText();

    // 布局
    QHBoxLayout *ly = new QHBoxLayout(this);
    ly->setContentsMargins(0,0,0,0);
    ly->addWidget(m_btn);
    setLayout(ly);
    setFixedSize(380,70);

    initPopupUi();

    connect(m_btn, &QPushButton::clicked, this, &DateTimePicker::showPopup);

}

void DateTimePicker::initPopupUi()
{
    m_popup = new QDialog(this);
    m_popup->setModal(true);
    m_popup->setFixedSize(520, 460);
    m_popup->setStyleSheet("background-color: #ffffff; border-radius: 12px;");

    // 日历
    m_calendar = new QCalendarWidget;
    m_calendar->setSelectedDate(m_currentDateTime.date());

    // 时间
    m_spinHour = new QSpinBox;
    m_spinMinute = new QSpinBox;
    m_spinHour->setRange(0,23);
    m_spinMinute->setRange(0,59);
    m_spinHour->setValue(m_currentDateTime.time().hour());
    m_spinMinute->setValue(m_currentDateTime.time().minute());

    QFont spinFont("Microsoft YaHei", 20);
    m_spinHour->setFont(spinFont);
    m_spinMinute->setFont(spinFont);
    m_spinHour->setFixedSize(100,60);
    m_spinMinute->setFixedSize(100,60);

    QLabel *lab1 = new QLabel("时");
    QLabel *lab2 = new QLabel("分");
    lab1->setFont(spinFont);
    lab2->setFont(spinFont);

    QHBoxLayout *timeLy = new QHBoxLayout;
    timeLy->addStretch();
    timeLy->addWidget(m_spinHour);
    timeLy->addWidget(lab1);
    timeLy->addWidget(m_spinMinute);
    timeLy->addWidget(lab2);
    timeLy->addStretch();

    // 按钮
    QPushButton *btnOk = new QPushButton("确认");
    QPushButton *btnCancel = new QPushButton("取消");
    btnOk->setFixedHeight(50);
    btnCancel->setFixedHeight(50);
    btnOk->setStyleSheet(R"(
        QPushButton{background:#3498db;color:white;border-radius:6px;font-size:20px;}
        QPushButton:hover{background:#2980b9;}
    )");
    btnCancel->setStyleSheet(R"(
        QPushButton{background:#e0e0e0;color:#333;border-radius:6px;font-size:20px;}
        QPushButton:hover{background:#bdc3c7;}
    )");

    QHBoxLayout *btnLy = new QHBoxLayout;
    btnLy->addStretch();
    btnLy->addWidget(btnOk);
    btnLy->addWidget(btnCancel);
    btnLy->addStretch();

    // 总布局
    QVBoxLayout *mainLy = new QVBoxLayout(m_popup);
    mainLy->addWidget(m_calendar);
    mainLy->addLayout(timeLy);
    mainLy->addLayout(btnLy);
    mainLy->setContentsMargins(20,20,20,20);
    mainLy->setSpacing(16);

    connect(btnOk, &QPushButton::clicked, this, &DateTimePicker::onConfirm);
    connect(btnCancel, &QPushButton::clicked, this, &DateTimePicker::onCancel);
}

void DateTimePicker::setDateTime(const QDateTime &dt)
{
    if (dt.isValid() && dt != m_currentDateTime) {
        m_currentDateTime = dt;
        updateButtonText();
        emit dateTimeChanged(m_currentDateTime);
    }
}

QDateTime DateTimePicker::dateTime() const
{
    return m_currentDateTime;
}

void DateTimePicker::updateButtonText()
{
    m_btn->setText(m_currentDateTime.toString("yyyy-MM-dd HH:mm"));
}

void DateTimePicker::showPopup()
{
    QPoint g = mapToGlobal(pos());
    int x = g.x() + width()/2 - m_popup->width()/2;
    int y = g.y() + height() + 10;
    m_popup->move(x, y);

    m_calendar->setSelectedDate(m_currentDateTime.date());
    m_spinHour->setValue(m_currentDateTime.time().hour());
    m_spinMinute->setValue(m_currentDateTime.time().minute());

    m_popup->exec();
}

void DateTimePicker::onConfirm()
{
    QDate d = m_calendar->selectedDate();
    QTime t(m_spinHour->value(), m_spinMinute->value());
    QDateTime dt(d, t);

    if (dt.isValid()) {
        setDateTime(dt);
    }
    m_popup->close();
}

void DateTimePicker::onCancel()
{
    m_popup->close();
}

