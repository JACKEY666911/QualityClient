#ifndef QUALITYCONTROLVIEW_H
#define QUALITYCONTROLVIEW_H

#include "HistoryMainView.h"
#include "QualityControlBaseView.h"
#include "untils/timecounter.h"
#include "Models/ImageDistributeInfo.h"

#include <QStringList>
#include <QVector>

class QLabel;
class QPushButton;
class SwitchButton;
class QTimer;
class QPixmap;
class QStackedWidget;
class QualityControlService;
class HistoryMainViewWidget;
class QualityControlHistoryView;
class PersonBaggageView;

class QualityControlView : public QualityControlBaseView
{
    Q_OBJECT
public:
    explicit QualityControlView(QWidget *parent = nullptr);

    void setUserName(const QString &userName);
    void setTaskCount(int count);
    void setCountdownText(const QString &text);
    void setMainImage(const QPixmap &pixmap);
    void setAuxImage(const QPixmap &pixmap);
    void setImageDistributeInfo(const ImageDistributeInfo &info);
    void clearImageDistributeInfo();

signals:
    void requestModeSwitch();
    void requestTaskStats();
    void requestHistory();
    void taskPauseToggled(bool paused);
    void requestNextImageDistributeInfo();

private slots:
    void handleStartCheckClicked();
    void handlePassClicked();
    void handleServiceError(const QString &message);
    void handlePauseToggled(bool checked);
    void handleDurationTick(const int sec);
    void handleDurationFinish();
    void openHistoryView();
    void closeHistoryMainView();
    void openHistoryDetailView(const ImageDistributeInfo &info);
    void closeHistoryView();
    void openPersonBaggageView();
    void closePersonBaggageView();

private:
    enum class PullState {
        Paused,
        WaitingTask,
        DisplayingTask
    };

    QWidget *buildTopBar() override;
    void loadNextImage();
    void initTestLayerItems();
    void cacheAndPauseForSubView();
    void restorePauseAfterSubView();
    void resetSubViewsForNewImage();
    void switchToHomePage();
    bool switchToHistoryPage();
    bool switchToHistoryDetailPage(const ImageDistributeInfo &info);
    void switchToPersonBaggagePage();
    void updatePullState();

    QLabel *m_taskCountLabel;
    QLabel *m_durationTitleLabel;
    QLabel *m_durationDot;
    QLabel *m_durationValueLabel;
    QLabel *m_userLabel;
    QLabel *m_userName;
    SwitchButton *m_pauseToggle;
    QPushButton *m_statsButton;
    QPushButton *m_historyButton;
    QPushButton *m_switchModeButton;
    QStackedWidget *m_navStack;
    QWidget *m_homePage;
    HistoryMainView *m_historyMainView;
    QualityControlHistoryView *m_historyView;
    PersonBaggageView *m_personBaggageView;

    struct SubViewSuspendState {
        bool pauseChecked = false;
        bool counterWasRunning = false;
        QWidget *returnPage = nullptr;
    };
    QVector<SubViewSuspendState> m_subViewSuspendStack;

    QStringList m_testImages;
    int m_testIndex;

    TimeCounter *m_secCounter;
    int m_elapsedSeconds;
    bool m_hasCurrentTask;
    PullState m_pullState;
    int m_consecutiveFetchErrors;
    ImageDistributeInfo m_currentDistributeInfo;

    QualityControlService *m_service;
};

#endif // QUALITYCONTROLVIEW_H
