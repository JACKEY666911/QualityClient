#ifndef MIXMODESELECTVIEW_H
#define MIXMODESELECTVIEW_H

#include <QAbstractButton>
#include <QFrame>
#include <QWidget>

namespace Ui {
class MixModeSelectView;
}

class MixModeSelectView : public QWidget
{
    Q_OBJECT

public:
    explicit MixModeSelectView(QWidget *parent = nullptr);
    ~MixModeSelectView();

    enum TimeRangeMode
    {
        Realtime = 1,   // 实时质控
        Interval = 2,    // 时段质控
        Normal = 3      // 普通质控
    };

    enum NormalQualityMode
    {
        AscendingOrder = 0,//正序
        ReverseOrder = 1//倒序
    };

    enum IntervalQualityMode
    {
        Part = 1,//部分
        All = 2//全部
    };

signals:
    void loginOut();
    void enterQualityControl();
private slots:
    void on_loginoutButton_clicked();
    void on_switchToMainViewBtn_clicked();
    void on_loginOutBtn_clicked();
    void updateCurrentTime(); // 定义刷新时间的槽函数

    void on_retTimeBtn_clicked();

    void on_TimeSelectButton_3_clicked();

    void on_TimeSelectButton_5_clicked();

    void on_TimeSelectButton_10_clicked();
    void on_PartRadioBtn_clicked();

    void on_AllradioBtn_clicked();

    void on_AscendingOrderRadioBtn_clicked();

    void on_ReverseOrderRadioBtn_clicked();

    void on_realRadioButton_clicked();

    void on_IntervalRadioButton_clicked();

    void on_normalradioButton_clicked();

private:
    void setQuickTimeRange(int seconds);
    void updateModeUI(QAbstractButton* clickedBtn, TimeRangeMode mode);
private:
    Ui::MixModeSelectView *ui;
    QMap<QAbstractButton*, QFrame*> m_frameMap;
    QTimer *m_timeTimer; // 定义定时器指针
    TimeRangeMode m_selectedTimeRange;
    NormalQualityMode m_curNormalQualityMode;
    IntervalQualityMode m_curIntervalQualityMode;
};

#endif // MIXMODESELECTVIEW_H
