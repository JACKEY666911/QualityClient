#ifndef QUALITYCONTROLVIEW_H
#define QUALITYCONTROLVIEW_H

#include <QWidget>
#include <QList>
#include <QStringList>
#include "widgets/AnnotationGraphicsView.h"

class QLabel;
class QPushButton;
class QStackedWidget;
class ThumbnailWidget;
class QPixmap;
class SwitchButton;
class CollapsiblePanel;
class LayerViewWidget;
class QTimer;

class QualityControlView : public QWidget
{
    Q_OBJECT
public:
    explicit QualityControlView(QWidget *parent = nullptr);

    void setUserName(const QString &userName);
    void setTaskCount(int count);
    void setCountdownText(const QString &text);
    void setBrand(const QString &brandKey);
    void setMainImage(const QPixmap &pixmap);
    void setAuxImage(const QPixmap &pixmap);
    QList<AnnotationGraphicsView::AnnotationData> mainAnnotations() const;
    QList<AnnotationGraphicsView::AnnotationData> auxAnnotations() const;

signals:
    void requestModeSwitch();
    void requestTaskStats();
    void requestHistory();
    void taskPauseToggled(bool paused);

private slots:
    void handleViewActivated(AnnotationGraphicsView *view);
    void handlePauseToggled(bool checked);
    void handleDurationTick();

private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    QWidget *buildTopBar();
    QWidget *buildMiddleArea();
    QWidget *buildBottomBar();
    QWidget *buildBrandButtons(const QStringList &labels);
    void loadNextImage();
    void positionOverlayPanel();
    void updateOverlayGeometry();

    QLabel *m_taskCountLabel;
    QLabel *m_countdownLabel;
    QLabel *m_durationTitleLabel;
    QLabel *m_durationDot;
    QLabel *m_durationValueLabel;
    QLabel *m_userLabel;
    SwitchButton *m_pauseToggle;
    QPushButton *m_statsButton;
    QPushButton *m_historyButton;
    QPushButton *m_switchModeButton;

    AnnotationGraphicsView *m_mainView;
    AnnotationGraphicsView *m_auxView;
    AnnotationGraphicsView *m_activeView;

    QStackedWidget *m_brandStack;
    ThumbnailWidget *m_thumbnail;
    QWidget *m_middleArea;
    CollapsiblePanel *m_overlayPanel;
    LayerViewWidget *m_layerView;

    QStringList m_testImages;
    int m_testIndex;

    QTimer *m_durationTimer;
    int m_elapsedSeconds;
};

#endif // QUALITYCONTROLVIEW_H
