#ifndef PERSONBAGGAGEVIEWWIDGE_H
#define PERSONBAGGAGEVIEWWIDGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QPixmap>
#include <QString>

// ===================== 拆分后的子结构体 =====================
/**
 * 安检信息结构体
 * 仅包含安检相关字段
 */
struct SecurityCheckData {
    QString passengerName;  // 旅客姓名
    QString flightNo;       // 航班号
    QString seatNo;         // 座位号
    QString destination;    // 目的地
    QString flightDate;     // 航班日期
    QString checkChannel;   // 安检通道
    QString checkTime;      // 安检时间
    QPixmap portraitPix;    // 人像图
};

/**
 * 质控信息结构体
 * 仅包含质控相关字段
 */
struct QualityControlData {
    QString qcChannel;      // 质控通道
    QString imageSource;    // 图像来源
    QString judgeTime;      // 判图时间
    QString judgeResult;    // 判图结果
    bool isJudgePass = false;  // 判图结果是否为放行（控制颜色）
    QString aiResult;       // AI结果
    bool isAiSuspicious = false; // AI结果是否为可疑（控制颜色）
    QString qcTime;         // 质控时间
    QString qcResult;       // 质控结果
    QPixmap qcXrayPix;      // 质控X光图
};

/**
 * 人包对应信息结构体
 * 仅包含人包对应相关字段
 */
struct PersonBaggageRelData {
    QPixmap baggagePortraitPix; // 旅客头像
    QPixmap bagAppearancePix;   // 行李外观图
    QPixmap bagXrayPix;         // 过检X光图
    int currentPage = 1;        // 当前页码
    int totalPage = 3;          // 总页数
};

/**
 * 顶层聚合结构体
 * 整合所有子结构体，支持单函数传参
 */
struct PersonBaggageData {
    SecurityCheckData securityData;    // 安检信息
    QualityControlData qualityData;    // 质控信息
    PersonBaggageRelData baggageData;  // 人包对应信息
};

class PersonBaggageViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PersonBaggageViewWidget(QWidget *parent = nullptr);
    ~PersonBaggageViewWidget() override = default;

    // 核心更新函数：单个函数传入所有数据（聚合结构体）
    void updateData(const PersonBaggageData &data);

    // 可选：单独更新某一部分数据的接口（按需扩展）
    void updateSecurityData(const SecurityCheckData &data);
    void updateQualityData(const QualityControlData &data);
    void updateBaggageData(const PersonBaggageRelData &data);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // 初始化UI
    void initUI();
    void initTopNavigationBar();
    void initSecurityCheckCard();
    void initQualityControlCard();
    void initPersonBaggageCard();

    // 成员变量（UI控件）
    QWidget *m_topBar = nullptr;

    // 安检信息卡片
    QFrame *m_securityCard = nullptr;
    QLabel *m_lblPassengerName = nullptr;
    QLabel *m_lblFlightNo = nullptr;
    QLabel *m_lblSeatNo = nullptr;
    QLabel *m_lblDestination = nullptr;
    QLabel *m_lblFlightDate = nullptr;
    QLabel *m_lblCheckChannel = nullptr;
    QLabel *m_lblCheckTime = nullptr;
    QLabel *m_lblPortrait = nullptr;

    // 质控信息卡片
    QFrame *m_qualityCard = nullptr;
    QLabel *m_lblQcChannel = nullptr;
    QLabel *m_lblImageSource = nullptr;
    QLabel *m_lblJudgeTime = nullptr;
    QLabel *m_lblJudgeResult = nullptr;
    QLabel *m_lblAiResult = nullptr;
    QLabel *m_lblQcTime = nullptr;
    QLabel *m_lblQcResult = nullptr;
    QLabel *m_lblQcXray = nullptr;

    // 人包对应信息卡片
    QFrame *m_baggageCard = nullptr;
    QLabel *m_lblBaggagePortrait = nullptr;
    QLabel *m_lblBagAppearance = nullptr;
    QLabel *m_lblBagXray = nullptr;
    QLabel *m_lblPageInfo = nullptr;
    int m_currentPage = 1;
    int m_totalPage = 3;
};

#endif // PERSONBAGGAGEVIEWWIDGE_H
