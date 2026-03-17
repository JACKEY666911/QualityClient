#include "DateTimePicker.h"
#include <QCalendarWidget>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QSpinBox>
#include <QTextCharFormat>
#include <QToolButton>
#include <QVBoxLayout>
#include <QPushButton>

DateTimePicker::DateTimePicker(QWidget *parent)
    : QWidget(parent)
    , m_display(nullptr)
    , m_dropButton(nullptr)
    , m_popup(nullptr)
    , m_calendar(nullptr)
    , m_hourList(nullptr)
    , m_minList(nullptr)
    , m_secList(nullptr)
    , m_suppressTimeSync(false)
{
    m_currentDateTime = QDateTime::currentDateTime();

    // 显示框
    m_display = new QLineEdit(this);
    m_display->setReadOnly(true);
    m_display->setFixedHeight(32);
    m_display->setCursor(Qt::PointingHandCursor);
    m_display->setStyleSheet(
        "QLineEdit{"
        "   background:white;"
        "   border:1px solid #c9c9c9;"
        "   border-radius:4px;"
        "   padding:4px 8px;"
        "   font-size:14px;"
        "}"
        );
    m_display->installEventFilter(this);

    // 下拉按钮
    m_dropButton = new QToolButton(this);
    m_dropButton->setFixedSize(24, 24);
    m_dropButton->setText("▾");
    m_dropButton->setStyleSheet(
        "QToolButton{"
        "   background:#0f6d89;"
        "   color:white;"
        "   border:none;"
        "   border-radius:4px;"
        "}"
        "QToolButton:hover{"
        "   background:#1382a3;"
        "}"
        );

    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(6);
    mainLayout->addWidget(m_display, 1);
    mainLayout->addWidget(m_dropButton);
    setLayout(mainLayout);
    setFixedWidth(260);

    initPopupUi();
    updateDisplayText();

    connect(m_dropButton, &QToolButton::clicked, this, &DateTimePicker::showPopup);
}

bool DateTimePicker::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_display && event->type() == QEvent::MouseButtonPress) {
        showPopup();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void DateTimePicker::initPopupUi()
{
    // 弹出框
    m_popup = new QFrame(this, Qt::Popup);
    m_popup->setStyleSheet(
        "QFrame{"
        "   background:#ffffff;"
        "   border:1px solid #cfcfcf;"
        "   border-radius:6px;"
        "}"
        );
    m_popup->setFixedSize(700, 320);

    // 日历控件 - 去掉所有边框
    m_calendar = new QCalendarWidget(m_popup);
    m_calendar->setFixedSize(420, 260);
    m_calendar->setGridVisible(true);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_calendar->setHorizontalHeaderFormat(QCalendarWidget::NoHorizontalHeader);
    m_calendar->setNavigationBarVisible(true);
    m_calendar->setStyleSheet(
        "QCalendarWidget{"
        "   border:none;"
        "   background-color:#FFFFFF;"
        "}"
        "QCalendarWidget QWidget{"
        "   border:none;"
        "}"
        "QCalendarWidget QWidget#qt_calendar_navigationbar{"
        "   background:#ffffff;"
        "   border-bottom:1px solid #e6e6e6;"
        "}"
        "QCalendarWidget QToolButton{"
        "   background:#ffffff;"
        "   color:#000000;"
        "   border:none;"
        "   font-weight:bold;"
        "}"
        "QCalendarWidget QToolButton:hover{"
        "   color:#1e8caa;"
        "}"
        "QCalendarWidget QSpinBox#qt_calendar_yearedit{"
        "   background:#ffffff;"
        "   height:30px;"
        "   width:90px;"
        "   selection-background-color:#1e8caa;"
        "   border:none;"
        "}"
        "QCalendarWidget QAbstractItemView{"
        "   font-size:11px;"
        "   gridline-color:#e6e6e6;"
        "}"
        "QCalendarWidget QAbstractItemView::item{"
        "   padding:2px;"
        "}"
        "QCalendarWidget QAbstractItemView:enabled{"
        "   color:#000000;"
        "   background-color:#ffffff;"
        "   selection-color:white;"
        "   selection-background-color:#1e8caa;"
        "   border:none;"
        "   outline:none;"
        "}"
        "QCalendarWidget QSpinBox#qt_calendar_yearedit{"
        "   background:#ffffff;"
        "   height:34px;"
        "   width:125px;"
        "   selection-background-color:#1e8caa;"
        "   border:none;"
        "}"
        "QCalendarWidget QToolButton{"
        "   background-color:#FFFFFF;"
        "   height:34px;"
        "   width:125px;"
        "   color:#000000;"
        "   border:none;"
        "}"
        "QCalendarWidget QToolButton:hover{"
        "   border:1px solid #1e8caa;"
        "}"
        "QCalendarWidget QTableView{"
        "   border:none;"
        "   outline:none;"
        "}"
        );

    // 设置日历内部控件
    const QList<QWidget*> calWidgets = m_calendar->findChildren<QWidget*>();
    if (!calWidgets.isEmpty() && calWidgets.first()) {
        calWidgets.first()->setCursor(Qt::PointingHandCursor);
    }

    const QList<QSpinBox*> spinBoxes = m_calendar->findChildren<QSpinBox*>();
    if (!spinBoxes.isEmpty() && spinBoxes.first()) {
        spinBoxes.first()->setAlignment(Qt::AlignCenter);
        spinBoxes.first()->setButtonSymbols(QSpinBox::NoButtons);
    }

    const QList<QToolButton*> toolBtns = m_calendar->findChildren<QToolButton*>();
    for (QToolButton *btn : toolBtns) {
        if (btn) {
            btn->setCursor(Qt::PointingHandCursor);
        }
    }

    const QSize iconSize(32, 32);
    if (QToolButton *prevBtn = m_calendar->findChild<QToolButton*>("qt_calendar_prevmonth")) {
        prevBtn->setIcon(QPixmap(":/left_ar.png"));
        prevBtn->setIconSize(iconSize);
    }
    if (QToolButton *nextBtn = m_calendar->findChild<QToolButton*>("qt_calendar_nextmonth")) {
        nextBtn->setIcon(QPixmap(":/right_ar.png"));
        nextBtn->setIconSize(iconSize);
    }

    // 时间面板
    QWidget *timePanel = new QWidget(m_popup);
    timePanel->setFixedWidth(210);
    timePanel->setStyleSheet("background:transparent;");

    QVBoxLayout *timeLayout = new QVBoxLayout(timePanel);
    timeLayout->setContentsMargins(10, 10, 10, 10);
    timeLayout->setSpacing(8);

    // 时分秒标题
    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->setSpacing(10);

    QLabel *hourLabel = new QLabel("时", timePanel);
    QLabel *minLabel = new QLabel("分", timePanel);
    QLabel *secLabel = new QLabel("秒", timePanel);

    hourLabel->setAlignment(Qt::AlignCenter);
    minLabel->setAlignment(Qt::AlignCenter);
    secLabel->setAlignment(Qt::AlignCenter);
    hourLabel->setFixedWidth(60);
    minLabel->setFixedWidth(60);
    secLabel->setFixedWidth(60);

    headerLayout->addWidget(hourLabel);
    headerLayout->addWidget(minLabel);
    headerLayout->addWidget(secLabel);

    // 时分秒列表
    QHBoxLayout *listLayout = new QHBoxLayout;
    listLayout->setSpacing(10);

    m_hourList = new QListWidget(timePanel);
    m_minList = new QListWidget(timePanel);
    m_secList = new QListWidget(timePanel);

    m_hourList->setFixedWidth(60);
    m_minList->setFixedWidth(60);
    m_secList->setFixedWidth(60);

    // 去掉滚动条和边框
    m_hourList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_minList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_secList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_hourList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_minList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_secList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_hourList->setStyleSheet("QListWidget{border:none;background:transparent;}");
    m_minList->setStyleSheet("QListWidget{border:none;background:transparent;}");
    m_secList->setStyleSheet("QListWidget{border:none;background:transparent;}");

    // 添加数字
    for (int i = 0; i < 24; ++i) {
        m_hourList->addItem(QString("%1").arg(i, 2, 10, QLatin1Char('0')));
    }
    for (int i = 0; i < 60; ++i) {
        QString value = QString("%1").arg(i, 2, 10, QLatin1Char('0'));
        m_minList->addItem(value);
        m_secList->addItem(value);
    }

    // 数字居中
    for (int i = 0; i < m_hourList->count(); ++i) {
        m_hourList->item(i)->setTextAlignment(Qt::AlignCenter);
    }
    for (int i = 0; i < m_minList->count(); ++i) {
        m_minList->item(i)->setTextAlignment(Qt::AlignCenter);
        m_secList->item(i)->setTextAlignment(Qt::AlignCenter);
    }

    listLayout->addWidget(m_hourList);
    listLayout->addWidget(m_minList);
    listLayout->addWidget(m_secList);

    // 确定按钮
    timeLayout->addLayout(headerLayout);
    timeLayout->addLayout(listLayout);
    timeLayout->addStretch(1);

    // 弹出框主布局
    QHBoxLayout *popupLayout = new QHBoxLayout(m_popup);
    popupLayout->setContentsMargins(12, 12, 12, 12);
    popupLayout->setSpacing(15);
    popupLayout->addWidget(m_calendar);
    popupLayout->addWidget(timePanel);

    // 连接信号
    connect(m_calendar, &QCalendarWidget::selectionChanged, this, [this]() {
        handleDateChanged(m_calendar->selectedDate());
    });
    connect(m_hourList, &QListWidget::currentRowChanged, this, [this](int) { handleTimeChanged(); });
    connect(m_minList, &QListWidget::currentRowChanged, this, [this](int) { handleTimeChanged(); });
    connect(m_secList, &QListWidget::currentRowChanged, this, [this](int) { handleTimeChanged(); });
}

void DateTimePicker::setDateTime(const QDateTime &dt)
{
    if (dt.isValid() && dt != m_currentDateTime) {
        m_currentDateTime = dt;
        updateDisplayText();
        updatePopupSelection();
        emit dateTimeChanged(m_currentDateTime);
    }
}

QDateTime DateTimePicker::dateTime() const
{
    return m_currentDateTime;
}

void DateTimePicker::updateDisplayText()
{
    if (m_display) {
        m_display->setText(m_currentDateTime.toString("yyyy-MM-dd HH:mm:ss"));
    }
}

void DateTimePicker::updatePopupSelection()
{
    if (!m_calendar || !m_hourList || !m_minList || !m_secList) {
        return;
    }

    m_calendar->setSelectedDate(m_currentDateTime.date());

    if (!m_suppressTimeSync) {
        m_hourList->setCurrentRow(m_currentDateTime.time().hour());
        m_minList->setCurrentRow(m_currentDateTime.time().minute());
        m_secList->setCurrentRow(m_currentDateTime.time().second());

        if (m_hourList->currentItem()) {
            m_hourList->scrollToItem(m_hourList->currentItem(), QAbstractItemView::PositionAtCenter);
        }
        if (m_minList->currentItem()) {
            m_minList->scrollToItem(m_minList->currentItem(), QAbstractItemView::PositionAtCenter);
        }
        if (m_secList->currentItem()) {
            m_secList->scrollToItem(m_secList->currentItem(), QAbstractItemView::PositionAtCenter);
        }
    }
}

void DateTimePicker::showPopup()
{
    if (!m_popup) {
        return;
    }

    updatePopupSelection();

    QPoint globalPos = mapToGlobal(QPoint(0, height()));
    m_popup->move(globalPos.x(), globalPos.y() + 4);
    m_popup->show();
}

void DateTimePicker::handleDateChanged(const QDate &date)
{
    QTime time = m_currentDateTime.time();
    setDateTime(QDateTime(date, time));
}

void DateTimePicker::handleTimeChanged()
{
    if (!m_hourList || !m_minList || !m_secList) {
        return;
    }

    if (m_hourList->currentRow() < 0 || m_minList->currentRow() < 0 || m_secList->currentRow() < 0) {
        return;
    }

    int hour = m_hourList->currentRow();
    int minute = m_minList->currentRow();
    int second = m_secList->currentRow();
    QDate date = m_currentDateTime.date();

    m_suppressTimeSync = true;
    setDateTime(QDateTime(date, QTime(hour, minute, second)));
    m_suppressTimeSync = false;
}
