#ifndef QUALITYCONTROLVIEW_H
#define QUALITYCONTROLVIEW_H

#include "QualityControlBaseView.h"
#include "untils/timecounter.h"
#include "Models/ImageDistributeInfo.h"

#include <QStringList>

class QLabel;
class QPushButton;
class SwitchButton;
class QTimer;
class QPixmap;
class QualityControlService;
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
    QualityControlHistoryView *m_historyView;
    PersonBaggageView *m_personBaggageView;

    QStringList m_testImages;
    int m_testIndex;

    TimeCounter *m_secCounter;
    int m_elapsedSeconds;
    bool m_cachedPauseChecked;
    bool m_cachedCounterRunning;
    bool m_hasCurrentTask;
    PullState m_pullState;
    int m_consecutiveFetchErrors;
    ImageDistributeInfo m_currentDistributeInfo;

    QualityControlService *m_service;
};

#endif // QUALITYCONTROLVIEW_H
