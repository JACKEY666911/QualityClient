#ifndef QUALITYCONTROLHISTORYVIEW_H
#define QUALITYCONTROLHISTORYVIEW_H

#include "QualityControlBaseView.h"
#include "Models/ImageDistributeInfo.h"

class QPushButton;
class QLabel;
class QualityControlHistoryService;

class QualityControlHistoryView : public QualityControlBaseView
{
    Q_OBJECT
public:
    explicit QualityControlHistoryView(QWidget *parent = nullptr);
    void setImageDistributeInfo(const ImageDistributeInfo &info);
    void clearImageDistributeInfo();

signals:
    void requestBack();

private:
    QWidget *buildTopBar() override;

    QPushButton *m_backButton;
    QLabel *m_titleLabel;
    ImageDistributeInfo m_currentDistributeInfo;
    QualityControlHistoryService *m_service;
};

#endif // QUALITYCONTROLHISTORYVIEW_H
