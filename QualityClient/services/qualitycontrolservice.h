#pragma once

#include "services/qualitycontrolservicebase.h"
#include "Models/ImageDistributeInfo.h"

class QTimer;

class QualityControlService : public QualityControlServiceBase
{
    Q_OBJECT
public:
    explicit QualityControlService(QObject *parent = nullptr);

signals:
    void imageDistributeInfoReceived(const ImageDistributeInfo &info);
    void noTaskAvailable();

public slots:
    void startPolling();
    void setPollingEnabled(bool enabled);
    void fetchNextTaskNow();
    void startCheck() override;
    void pass() override;

private:
    void pullTask(bool forceNow);

    QTimer *m_pollTimer;
    bool m_pollingEnabled;
    bool m_requesting;
    bool m_firstFetch;
    QString m_lastTaskId;
    QString m_deviceCode;
};
