#include "MixModeSelectView.h"
#include "ui_MixModeSelectView.h"

#include <QButtonGroup>
#include <QDebug>
#include <QTimer>

#include <widgets/CheckComBoBox.h>
// #include "widgets/moderndatetimepicker.h"
MixModeSelectView::MixModeSelectView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MixModeSelectView)
{
    ui->setupUi(this);
    //定时器相关
    m_timeTimer = new QTimer(this);
    connect(m_timeTimer, &QTimer::timeout, this, &MixModeSelectView::updateCurrentTime);
    m_timeTimer->start(1000);
    updateCurrentTime();

    m_selectedTimeRange = MixModeSelectView::Realtime;

    QButtonGroup *radioGroup = new QButtonGroup(this);
    m_frameMap[ui->realRadioButton] = ui->realFrame;
    m_frameMap[ui->IntervalRadioButton] = ui->IntervalFrame;
    m_frameMap[ui->normalradioButton] = ui->normalFrame;
    radioGroup->addButton(ui->realRadioButton, 1);
    radioGroup->addButton(ui->IntervalRadioButton, 2);
    radioGroup->addButton(ui->normalradioButton, 3);
    // connect(radioGroup, qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked),
    //         this, [=](QAbstractButton *btn) {
    //     for (auto it = m_frameMap.begin(); it != m_frameMap.end(); ++it) {
    //         QFrame* target = it.value();
    //         bool isSelected = (it.key() == btn);

    //         target->setProperty("selected", isSelected);
    //         target->style()->unpolish(target);
    //         target->style()->polish(target);
    //     }
    //     qDebug() << "1111" << btn->text();
    // });
    QList<AreaInfo> list;
    list << AreaInfo(1, "北京")
         << AreaInfo(2, "上海")
         << AreaInfo(3, "深圳");
    ui->realCheckComboBox->setAreas(list);
    ui->IntervalCheckComboBox->setAreas(list);
}

MixModeSelectView::~MixModeSelectView()
{
    delete ui;
}

void MixModeSelectView::on_loginoutButton_clicked()
{
    emit loginOut();
}

void MixModeSelectView::on_switchToMainViewBtn_clicked()
{
    emit enterQualityControl();
}

void MixModeSelectView::on_loginOutBtn_clicked()
{
    emit loginOut();
}

void MixModeSelectView::updateCurrentTime()
{
    // 获取当前系统时间
    QDateTime currentTime = QDateTime::currentDateTime();

    // 格式化时间字符串：年-月-日 时:分:秒
    // "yyyy-MM-dd hh:mm:ss" -> 24小时制用 hh，12小时制用 h
    QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");

    // 显示到 Label 上
    ui->timeLabel->setText(timeStr);
}


void MixModeSelectView::on_retTimeBtn_clicked()
{
    ui->startTime->setDateTime(QDateTime());
    ui->endTime->setDateTime(QDateTime());
}


void MixModeSelectView::on_TimeSelectButton_3_clicked()
{
    setQuickTimeRange(180);
}


void MixModeSelectView::on_TimeSelectButton_5_clicked()
{
    setQuickTimeRange(300);
}


void MixModeSelectView::on_TimeSelectButton_10_clicked()
{
    setQuickTimeRange(600);
}

void MixModeSelectView::setQuickTimeRange(int seconds)
{
    QDateTime now = QDateTime::currentDateTime();
    ui->endTime->setDateTime(now);
    ui->startTime->setDateTime(now.addSecs(-seconds));
}

void MixModeSelectView::updateModeUI(QAbstractButton *clickedBtn, TimeRangeMode mode)
{
    // 1. 更新枚举状态
    m_selectedTimeRange = mode;

    // 2. 遍历 Map 更新 Frame 高亮状态
    for (auto it = m_frameMap.begin(); it != m_frameMap.end(); ++it) {
        QFrame* target = it.value();
        bool isSelected = (it.key() == clickedBtn);
        target->setProperty("selected", isSelected);
        target->style()->unpolish(target);
        target->style()->polish(target);
    }

    qDebug() << "Current Mode:" << mode;
}


void MixModeSelectView::on_PartRadioBtn_clicked()
{
    m_curIntervalQualityMode = Part;
}


void MixModeSelectView::on_AllradioBtn_clicked()
{
    m_curIntervalQualityMode = All;
}


void MixModeSelectView::on_AscendingOrderRadioBtn_clicked()
{
    m_curNormalQualityMode = AscendingOrder;
}


void MixModeSelectView::on_ReverseOrderRadioBtn_clicked()
{
    m_curNormalQualityMode = ReverseOrder;
}


void MixModeSelectView::on_realRadioButton_clicked()
{
    updateModeUI(ui->realRadioButton, Realtime);
}


void MixModeSelectView::on_IntervalRadioButton_clicked()
{
    updateModeUI(ui->IntervalRadioButton, Interval);
}


void MixModeSelectView::on_normalradioButton_clicked()
{
    updateModeUI(ui->normalradioButton, Normal);
}

