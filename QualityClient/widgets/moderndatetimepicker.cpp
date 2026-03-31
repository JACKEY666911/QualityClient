#include "ModernDateTimePicker.h"
#include <QPainter>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QScreen>

// --- ModernTimeWheel (黄金比例版) ---
ModernTimeWheel::ModernTimeWheel(int min, int max, QWidget *parent)
    : QWidget(parent), m_min(min), m_max(max), m_val(min) {
    setFixedWidth(65);     // 宽度适中，不局促也不空旷
    setMinimumHeight(190); // 高度适中，显示 5 行数字
}

void ModernTimeWheel::setValue(int v) {
    m_val = qBound(m_min, v, m_max);
    update();
}

void ModernTimeWheel::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 选中态背景条
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 122, 255, 30));
    p.drawRoundedRect(5, height()/2 - 18, width() - 10, 36, 8, 8);

    for(int i = -2; i <= 2; ++i) {
        int cur = m_val + i;
        int range = m_max - m_min + 1;
        while(cur < m_min) cur += range;
        while(cur > m_max) cur -= range;

        double factor = 1.0 - qAbs(i) * 0.35;
        QColor col = (i == 0) ? QColor("#007AFF") : QColor("#606266");
        col.setAlphaF(qMax(0.12, factor));

        p.setPen(col);
        p.setFont(QFont("Microsoft YaHei", i == 0 ? 15 : 12, i == 0 ? QFont::Bold : QFont::Normal));

        // 行高 38，视觉重心稳健
        QRect r(0, height()/2 + i*38 - 19, width(), 38);
        p.drawText(r, Qt::AlignCenter, QString("%1").arg(cur, 2, 10, QChar('0')));
    }
}

void ModernTimeWheel::wheelEvent(QWheelEvent *e) {
    m_val += (e->angleDelta().y() > 0 ? -1 : 1);
    int range = m_max - m_min + 1;
    while (m_val < m_min) m_val += range;
    while (m_val > m_max) m_val -= range;
    update();
    emit valueChanged(m_val);
}

void ModernTimeWheel::mousePressEvent(QMouseEvent *e) {
    if (e->pos().y() < height()/2 - 18) m_val--;
    else if (e->pos().y() > height()/2 + 18) m_val++;
    int range = m_max - m_min + 1;
    while (m_val < m_min) m_val += range;
    while (m_val > m_max) m_val -= range;
    update();
    emit valueChanged(m_val);
}

// --- DateTimePickerPanel (平衡版) ---
DateTimePickerPanel::DateTimePickerPanel(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    m_selectedDate = QDate::currentDate();
    m_viewDate = m_selectedDate;
    setupUI();
    updateCalendar();
}

void DateTimePickerPanel::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    m_container = new QFrame();
    m_container->setObjectName("DateTimePickerMainFrame");
    m_container->setStyleSheet(
        "#DateTimePickerMainFrame { background: white; border-radius: 14px; border: 1px solid #E4E7ED; }"
        "QPushButton#NavBtn { border: none; background: #F5F7FA; border-radius: 7px; padding: 5px; font-weight: bold; min-width: 30px; font-size: 15px; }"
        "QPushButton#NavBtn:hover { background: #EBEEF5; color: #007AFF; }"
        );
    mainLayout->addWidget(m_container);

    auto *contentLayout = new QHBoxLayout(m_container);
    contentLayout->setContentsMargins(20, 20, 20, 20); // 适度的内边距
    contentLayout->setSpacing(25); // 左右模块的黄金间距

    // --- 左侧：日历 ---
    auto *calLayout = new QVBoxLayout();
    calLayout->setSpacing(12);

    auto *header = new QHBoxLayout();
    auto *btnYearPrev = new QPushButton("«");
    auto *btnMonthPrev = new QPushButton("<");
    auto *btnMonthNext = new QPushButton(">");
    auto *btnYearNext = new QPushButton("»");
    btnYearPrev->setObjectName("NavBtn"); btnMonthPrev->setObjectName("NavBtn");
    btnMonthNext->setObjectName("NavBtn"); btnYearNext->setObjectName("NavBtn");

    m_titleLabel = new QLabel();
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #303133;");

    header->addWidget(btnYearPrev); header->addWidget(btnMonthPrev);
    header->addStretch(); header->addWidget(m_titleLabel); header->addStretch();
    header->addWidget(btnMonthNext); header->addWidget(btnYearNext);
    calLayout->addLayout(header);

    auto *weekLayout = new QHBoxLayout();
    QStringList weeks = {"一", "二", "三", "四", "五", "六", "日"};
    for(const QString &w : weeks) {
        auto *l = new QLabel(w);
        l->setAlignment(Qt::AlignCenter);
        l->setStyleSheet("color: #909399; font-size: 13px; font-weight: bold;");
        weekLayout->addWidget(l);
    }
    calLayout->addLayout(weekLayout);

    m_calendarGrid = new QGridLayout();
    m_calendarGrid->setSpacing(5);
    for(int i=0; i<42; ++i) {
        auto *btn = new QPushButton();
        btn->setFixedSize(38, 38); // 38px 是兼顾开阔与节省空间的平衡点
        btn->setCursor(Qt::PointingHandCursor);
        m_calendarGrid->addWidget(btn, i/7, i%7);
        m_dateButtons.append(btn);
    }
    calLayout->addLayout(m_calendarGrid);
    contentLayout->addLayout(calLayout);

    // --- 右侧：时间设置 (醒目紧凑) ---
    auto *timeLayout = new QVBoxLayout();
    timeLayout->setSpacing(8);
    timeLayout->setContentsMargins(0, 0, 0, 0);

    auto *timeTitle = new QLabel("时间设置");
    timeTitle->setAlignment(Qt::AlignCenter);
    timeTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #303133;");
    timeTitle->setFixedHeight(25);

    auto *wheelLayout = new QHBoxLayout();
    wheelLayout->setSpacing(4);
    m_hour = new ModernTimeWheel(0, 23);
    m_min = new ModernTimeWheel(0, 59);
    m_sec = new ModernTimeWheel(0, 59);
    wheelLayout->addWidget(m_hour);
    wheelLayout->addWidget(m_min);
    wheelLayout->addWidget(m_sec);

    auto *btnConfirm = new QPushButton("确 认 选 择");
    btnConfirm->setCursor(Qt::PointingHandCursor);
    btnConfirm->setMinimumHeight(40);
    btnConfirm->setStyleSheet("QPushButton { background: #007AFF; color: white; border-radius: 8px; font-weight: bold; border: none; font-size: 14px; }"
                              "QPushButton:hover { background: #3395FF; }");

    timeLayout->addWidget(timeTitle);
    timeLayout->addLayout(wheelLayout);
    timeLayout->addSpacing(6);
    timeLayout->addWidget(btnConfirm);

    contentLayout->addLayout(timeLayout);

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(18); shadow->setColor(QColor(0,0,0,45)); shadow->setOffset(0,4);
    m_container->setGraphicsEffect(shadow);

    // 连接逻辑保持不变...
    connect(btnYearPrev, &QPushButton::clicked, [this]{ m_viewDate = m_viewDate.addYears(-1); updateCalendar(); });
    connect(btnMonthPrev, &QPushButton::clicked, [this]{ m_viewDate = m_viewDate.addMonths(-1); updateCalendar(); });
    connect(btnMonthNext, &QPushButton::clicked, [this]{ m_viewDate = m_viewDate.addMonths(1); updateCalendar(); });
    connect(btnYearNext, &QPushButton::clicked, [this]{ m_viewDate = m_viewDate.addYears(1); updateCalendar(); });
    connect(btnConfirm, &QPushButton::clicked, this, &DateTimePickerPanel::onConfirm);
}

void DateTimePickerPanel::updateCalendar() {
    m_titleLabel->setText(m_viewDate.toString("yyyy年 MM月"));
    QDate firstDay(m_viewDate.year(), m_viewDate.month(), 1);
    int offset = firstDay.dayOfWeek() - 1;
    QDate startDate = firstDay.addDays(-offset);

    for (int i = 0; i < 42; ++i) {
        QDate d = startDate.addDays(i);
        QPushButton* btn = m_dateButtons[i];
        btn->setText(QString::number(d.day()));
        btn->disconnect();
        connect(btn, &QPushButton::clicked, [this, d]{ m_selectedDate = d; updateCalendar(); });

        QString style = "border:none; border-radius:19px; font-size:14px; ";
        if (d.month() != m_viewDate.month()) {
            style += "color: #DCDFE6; background: transparent;";
        } else if (d == m_selectedDate) {
            style += "background: #007AFF; color: white; font-weight: bold;";
        } else if (d == QDate::currentDate()) {
            style += "border: 1px solid #007AFF; color: #007AFF; background: #F0F7FF;";
        } else {
            style += "color: #606266; background: transparent;";
        }
        btn->setStyleSheet(style);
    }
}

void DateTimePickerPanel::setDateTime(const QDateTime &dt) {
    if (!dt.isValid()) return;
    m_selectedDate = dt.date();
    m_viewDate = m_selectedDate;
    m_hour->setValue(dt.time().hour());
    m_min->setValue(dt.time().minute());
    m_sec->setValue(dt.time().second());
    updateCalendar();
}

void DateTimePickerPanel::onConfirm() {
    QDateTime dt(m_selectedDate, QTime(m_hour->value(), m_min->value(), m_sec->value()));
    emit dateTimeChanged(dt);
    hide();
}

void DateTimePickerPanel::showEvent(QShowEvent *e) {
    QWidget::showEvent(e);
    setFocus();
}

// --- ModernDateTimeEdit ---
ModernDateTimeEdit::ModernDateTimeEdit(QWidget *parent) : QLineEdit(parent) {
    setReadOnly(true);
    setPlaceholderText("请选择日期时间...");
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);

    setStyleSheet("QLineEdit { border: 1px solid #DCDFE6; border-radius: 7px; padding: 9px 28px 9px 12px; background: white; color: #333; font-size: 15px; }"
                  "QLineEdit:hover { border-color: #409EFF; }");

    m_panel = new DateTimePickerPanel(nullptr);
    m_panel->installEventFilter(this);
    connect(m_panel, &DateTimePickerPanel::dateTimeChanged, this, &ModernDateTimeEdit::setDateTime);
}

void ModernDateTimeEdit::setDateTime(const QDateTime &dt) {
    m_dt = dt;
    if (dt.isValid()) {
        setText(dt.toString("yyyy-MM-dd HH:mm:ss"));
        deselect();
    } else {
        clear();
    }
}

bool ModernDateTimeEdit::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_panel && event->type() == QEvent::Hide) {
        m_lastHideTime = QDateTime::currentMSecsSinceEpoch();
    }
    return QLineEdit::eventFilter(watched, event);
}

void ModernDateTimeEdit::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if (currentTime - m_lastHideTime < 250) return;

        if (m_panel->isVisible()) {
            m_panel->hide();
        } else {
            m_panel->setDateTime(m_dt.isValid() ? m_dt : QDateTime::currentDateTime());
            QPoint pos = mapToGlobal(QPoint(0, height() + 4));
            if (pos.y() + m_panel->sizeHint().height() > screen()->availableGeometry().bottom())
                pos = mapToGlobal(QPoint(0, -m_panel->sizeHint().height() - 4));
            m_panel->move(pos);
            m_panel->show();
        }
    }
}

void ModernDateTimeEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    mousePressEvent(e);
}

void ModernDateTimeEdit::paintEvent(QPaintEvent *e) {
    QLineEdit::paintEvent(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor("#C0C4CC"), 1.8));
    QRect r = rect().adjusted(width()-25, 11, -11, -11);
    p.drawRoundedRect(r, 1.5, 1.5);
    p.drawLine(r.left(), r.top()+4, r.right(), r.top()+4);
}