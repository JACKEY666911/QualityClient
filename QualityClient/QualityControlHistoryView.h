#ifndef QUALITYCONTROLHISTORYVIEW_H
#define QUALITYCONTROLHISTORYVIEW_H

#include "QualityControlBaseView.h"

class QPushButton;
class QLabel;
class QualityControlHistoryService;
class QualityControlController;

class QualityControlHistoryView : public QualityControlBaseView
{
    Q_OBJECT
public:
    explicit QualityControlHistoryView(QWidget *parent = nullptr);

signals:
    void requestBack();

private:
    QWidget *buildTopBar() override;

    QPushButton *m_backButton;
    QLabel *m_titleLabel;
    QualityControlHistoryService *m_service;
    QualityControlController *m_controller;
};

#endif // QUALITYCONTROLHISTORYVIEW_H
