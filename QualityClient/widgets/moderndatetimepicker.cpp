#include "ModernDateTimePicker.h"
#include <QPainter>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QScreen>
#include <QStyle>

// --- ModernTimeWheel Implementation ---
ModernTimeWheel::ModernTimeWheel(int min, int max, QWidget *parent)
    : QWidget(parent), m_min(min), m_max(max) {
    m_val = min;
    setFixedWidth(45);
    setMinimumHeight(150);
}

void ModernTimeWheel::setValue(int v) {
    m_val = v;
    update();
}

void ModernTimeWheel::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    int w = width();
    int h = height();

    for(int i = -2; i <= 2; ++i)
    {
        int cur = m_val + i;
        int range = m_max - m_min + 1;
        while(cur < m_min) cur += range;
        while(cur > m_max) cur -= range;

        double opacity = 1.0 - qAbs(i) * 0.4;
        if(opacity < 0) opacity = 0;
        p.setOpacity(opacity);//设置透明度
        p.setPen(i == 0 ? QColor(tr("#007AFF")): QColor(tr("#666666")));
        QFont f = p.font();
        f.setBold(i == 0);
        f.setPixelSize(i == 0 ? 15 :12);
        p.setFont(f);
        //还能设置字符宽度和补全字符
        p.drawText(QRect(0,h/2 + i*30 - 15, w ,30), Qt::AlignCenter, QString("%1").arg(cur, 2, 10, QChar('0')));
    }
    p.setOpacity(0.1);
    p.setBrush(QColor(tr("#007AFF")));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(2, h/2 - 15, w - 4, 30, 4, 4);
}

void ModernTimeWheel::wheelEvent(QWheelEvent *e) {
    m_val += (e->angleDelta().y() > 0 ? -1 : 1);
    int range = m_max - m_min + 1;
    while (m_val < m_min) m_val += range;
    while (m_val > m_max) m_val -= range;
    update();
    emit valueChanged(m_val);
}

void ModernTimeWheel::mousePressEvent(QMouseEvent *event)
{
    int clickY = static_cast<int>(event->position().y());

    // 点击逻辑：点击中间线（150/2=75）上方则减，下方则加
    // 这里的 15 是中心选中框高度的一半
    if (clickY < height() / 2 - 15) {
        m_val--;
    } else if (clickY > height() / 2 + 15) {
        m_val++;
    }

    int range = m_max - m_min + 1;
    while (m_val < m_min) m_val += range;
    while (m_val > m_max) m_val -= range;

    update();
    emit valueChanged(m_val);
}

// --- DateTimePickerPanel Implementation ---
DateTimePickerPanel::DateTimePickerPanel(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint| Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    m_selectedDate = QDate::currentDate();
    m_viewDate = m_selectedDate;

    setupUI();
    updateCalendar();
}

void DateTimePickerPanel::setDateTime(const QDateTime &dt)
{
    if (!dt.isValid()) return;

    // 1. 更新日期逻辑
    m_selectedDate = dt.date();
    m_viewDate = m_selectedDate; // 确保视图切换到选中日期所在的月份

    // 2. 更新滚轮逻辑
    m_hour->setValue(dt.time().hour());
    m_min->setValue(dt.time().minute());
    m_sec->setValue(dt.time().second());

    // 3. 刷新日历 UI
    updateCalendar();
}

void DateTimePickerPanel::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    m_container = new QFrame();
    m_container->setObjectName("Container");
    m_container->setStyleSheet(
        "#Container { background: white; border-radius: 12px; border: 1px solid #DDD; }"
        "QPushButton#DateBtn { border: none; border-radius: 15px; background: transparent; padding: 5px; }"
        "QPushButton#DateBtn:hover { background: #F0F0F0; }"
        "QPushButton#NavBtn { border: none; background: #F5F5F5; border-radius: 4px; padding: 4px; font-weight: bold; width: 25px; }"
        "QPushButton#NavBtn:hover { background: #EAEAEA; color: #007AFF; }"
        );
    mainLayout->addWidget(m_container);

    auto *contentLayout = new QHBoxLayout(m_container);
    auto *calLayout = new QVBoxLayout();

    // --- 重新设计的 Header 布局 ---
    auto *header = new QHBoxLayout();
    header->setSpacing(5);

    // 左侧按钮组 (年、月)
    auto *btnYearPrev = new QPushButton("«"); // 快速切换年
    auto *btnMonthPrev = new QPushButton("<"); // 切换月
    btnYearPrev->setObjectName("NavBtn");
    btnMonthPrev->setObjectName("NavBtn");

    // 中间标签 (年份 月份)
    m_titleLabel = new QLabel();
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #333; min-width: 100px;");

    // 右侧按钮组 (月、年)
    auto *btnMonthNext = new QPushButton(">");
    auto *btnYearNext = new QPushButton("»");
    btnMonthNext->setObjectName("NavBtn");
    btnYearNext->setObjectName("NavBtn");

    header->addWidget(btnYearPrev);
    header->addWidget(btnMonthPrev);
    header->addStretch();
    header->addWidget(m_titleLabel);
    header->addStretch();
    header->addWidget(btnMonthNext);
    header->addWidget(btnYearNext);

    calLayout->addLayout(header);

    // --- 日历网格 ---
    m_calendarGrid = new QGridLayout();
    m_calendarGrid->setSpacing(2);
    calLayout->addLayout(m_calendarGrid);
    contentLayout->addLayout(calLayout, 3);

    // 分割线
    auto *line = new QFrame();
    line->setFixedWidth(1);
    line->setStyleSheet("background: #EEE; margin: 10px 0;");
    contentLayout->addWidget(line);

    // --- 时间滚轮部分 ---
    auto *timeLayout = new QVBoxLayout();
    auto *wheelLayout = new QHBoxLayout();
    m_hour = new ModernTimeWheel(0, 23);
    m_min = new ModernTimeWheel(0, 59);
    m_sec = new ModernTimeWheel(0, 59);
    wheelLayout->addWidget(m_hour);
    wheelLayout->addWidget(m_min);
    wheelLayout->addWidget(m_sec);

    auto *btnConfirm = new QPushButton("确 定");
    btnConfirm->setCursor(Qt::PointingHandCursor);
    btnConfirm->setStyleSheet("QPushButton { background: #007AFF; color: white; border-radius: 6px; padding: 7px; font-weight: bold; }"
                              "QPushButton:hover { background: #0062CC; }");

    timeLayout->addWidget(new QLabel("选择时间"), 0, Qt::AlignCenter);
    timeLayout->addLayout(wheelLayout);
    timeLayout->addWidget(btnConfirm);
    contentLayout->addLayout(timeLayout, 2);

    // 阴影效果
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 50));
    shadow->setOffset(0, 4);
    m_container->setGraphicsEffect(shadow);

    // --- 信号连接 ---
    connect(btnYearPrev, &QPushButton::clicked, [this]{ m_viewDate = m_viewDate.addYears(-1); updateCalendar(); });
    connect(btnMonthPrev, &QPushButton::clicked, [this]{ m_viewDate = m_viewDate.addMonths(-1); updateCalendar(); });
    connect(btnMonthNext, &QPushButton::clicked, [this]{ m_viewDate = m_viewDate.addMonths(1); updateCalendar(); });
    connect(btnYearNext, &QPushButton::clicked, [this]{ m_viewDate = m_viewDate.addYears(1); updateCalendar(); });
    connect(btnConfirm, &QPushButton::clicked, this, &DateTimePickerPanel::onConfirm);
}

void DateTimePickerPanel::updateCalendar() {
    QLayoutItem *item;
    while ((item = m_calendarGrid->takeAt(0)) != nullptr) {
        if(item->widget()) delete item->widget();
        delete item;
    }

    m_titleLabel->setText(m_viewDate.toString("yyyy年 MM月"));
    QDate firstDay(m_viewDate.year(), m_viewDate.month(), 1);
    int offset = firstDay.dayOfWeek() - 1;
    QDate startDate = firstDay.addDays(-offset);

    for (int i = 0; i < 42; ++i) {
        QDate d = startDate.addDays(i);
        auto *btn = new QPushButton(QString::number(d.day()));
        btn->setObjectName("DateBtn");
        btn->setFixedSize(30, 30);

        if (d.month() != m_viewDate.month()) {
            btn->setStyleSheet("color: #CCC;");
        } else if (d == m_selectedDate) {
            btn->setStyleSheet("background: #007AFF; color: white; font-weight: bold; border-radius: 15px;");
        } else if (d == QDate::currentDate()) {
            btn->setStyleSheet("border: 1px solid #007AFF; color: #007AFF; border-radius: 15px;");
        }
        connect(btn, &QPushButton::clicked, [this, d]{ m_selectedDate = d; updateCalendar(); });
        m_calendarGrid->addWidget(btn, i / 7, i % 7);
    }
}

void DateTimePickerPanel::onConfirm() {
    if (m_selectedDate.isNull()) {
        m_selectedDate = QDate::currentDate();
    }
    QDateTime dt(m_selectedDate, QTime(m_hour->value(), m_min->value(), m_sec->value()));
    emit dateTimeChanged(dt);
    this->hide();
}

// --- ModernDateTimeEdit Implementation ---
ModernDateTimeEdit::ModernDateTimeEdit(QWidget *parent) : QLineEdit(parent) {
    setReadOnly(true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    setPlaceholderText("请选择日期时间..."); // 添加占位符，增强体验
    m_dt = QDateTime();

    setStyleSheet("QLineEdit { border: 1px solid #DDD; border-radius: 6px; background: white; }"
                  "QLineEdit:hover { border-color: #007AFF; }");

    m_panel = new DateTimePickerPanel(nullptr);
    connect(m_panel, &DateTimePickerPanel::dateTimeChanged, this, &ModernDateTimeEdit::setDateTime);

    // setDateTime(m_dt);
}

void ModernDateTimeEdit::setDateTime(const QDateTime &dt) {
    m_dt = dt;
    if (dt.isValid()) {
        setText(dt.toString("yyyy-MM-dd HH:mm:ss"));
    } else {
        clear(); // 调用 QLineEdit 的清空
    }
}

void ModernDateTimeEdit::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        if (m_panel->isVisible()) {
            // m_panel->hide();
            // 如果当前没有值，则给面板传一个当前系统时间作为起始参考
            if (!m_dt.isValid()) {
                m_panel->setDateTime(QDateTime::currentDateTime());
            } else {
                m_panel->setDateTime(m_dt);
            }
        } else {
            // 关键：在弹出前，将输入框当前的 m_dt 同步给面板
            m_panel->setDateTime(m_dt);

            QPoint pos = mapToGlobal(QPoint(0, height() + 5));

            // 简单的防溢出处理
            if (pos.y() + m_panel->sizeHint().height() > screen()->availableGeometry().bottom()) {
                pos = mapToGlobal(QPoint(0, -m_panel->sizeHint().height() - 5));
            }

            m_panel->move(pos);
            m_panel->show();
            m_panel->raise();
            m_panel->activateWindow();
        }
        e->accept();
    }
}