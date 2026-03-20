#ifndef QUALITYCONTROLVIEW_H
#define QUALITYCONTROLVIEW_H

#include "QualityControlBaseView.h"

#include <QStringList>

class QLabel;
class QPushButton;
class SwitchButton;
class QTimer;
class QPixmap;
class QualityControlService;
class QualityControlController;

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

signals:
    void requestModeSwitch();
    void requestTaskStats();
    void requestHistory();
    void taskPauseToggled(bool paused);

private slots:
    void handlePauseToggled(bool checked);
    void handleDurationTick();

private:
    QWidget *buildTopBar() override;
    void loadNextImage();
    void initTestLayerItems();

    QLabel *m_taskCountLabel;
    QLabel *m_durationTitleLabel;
    QLabel *m_durationDot;
    QLabel *m_durationValueLabel;
    QLabel *m_userLabel;
    SwitchButton *m_pauseToggle;
    QPushButton *m_statsButton;
    QPushButton *m_historyButton;
    QPushButton *m_switchModeButton;

    QStringList m_testImages;
    int m_testIndex;

    QTimer *m_durationTimer;
    int m_elapsedSeconds;

    QualityControlService *m_service;
    QualityControlController *m_controller;
};

#endif // QUALITYCONTROLVIEW_H
