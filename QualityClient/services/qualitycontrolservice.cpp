#include "qualitycontrolservice.h"
#include "logging/logcategories.h"
#include "services/settingsservice.h"

#include <QFutureWatcher>
#include <QTimer>

QualityControlService::QualityControlService(QObject *parent)
    : QualityControlServiceBase(parent)
    , m_pollTimer(new QTimer(this))
    , m_pollingEnabled(false)
    , m_requesting(false)
    , m_firstFetch(true)
    , m_deviceCode(SettingsService::instance().deviceCode())
{
    qCInfo(lcQcService) << "QualityControlService deviceCode=" << m_deviceCode
                        << "airportCode=" << SettingsService::instance().airportCode()
                        << "areaCode=" << SettingsService::instance().areaCode();
    m_pollTimer->setInterval(2000);
    m_pollTimer->setSingleShot(true);
    connect(m_pollTimer, &QTimer::timeout, this, [this]() {
        pullTask(false);
    });
}

void QualityControlService::startPolling()
{
    setPollingEnabled(true);
}

void QualityControlService::setPollingEnabled(bool enabled)
{
    qCInfo(lcQcService) << "setPollingEnabled:" << enabled;
    m_pollingEnabled = enabled;
    if (!enabled) {
        m_pollTimer->stop();
        return;
    }

    pullTask(true);
}

void QualityControlService::fetchNextTaskNow()
{
    qCInfo(lcQcService) << "fetchNextTaskNow";
    pullTask(true);
}

void QualityControlService::startCheck()
{
    qCInfo(lcQcService) << "startCheck";
    emit statusMessage(QStringLiteral("quality control start check"));
}

void QualityControlService::pass()
{
    qCInfo(lcQcService) << "pass";
    emit statusMessage(QStringLiteral("quality control pass"));
}

void QualityControlService::pullTask(bool forceNow)
{
    if (m_requesting) {
        return;
    }
    if (!forceNow && !m_pollingEnabled) {
        return;
    }

    m_requesting = true;
    m_pollTimer->stop();

    ImageDistributeRequest request;
    request.id = m_lastTaskId;
    request.deviceCode = m_deviceCode;
    request.firstTime = m_firstFetch;

    QFutureWatcher<HttpClientAsync::HttpResponse> *watcher = new QFutureWatcher<HttpClientAsync::HttpResponse>(this);
    connect(watcher, &QFutureWatcher<HttpClientAsync::HttpResponse>::finished, this, [this, watcher]() {
        const HttpClientAsync::HttpResponse response = watcher->result();
        watcher->deleteLater();
        m_requesting = false;
        bool hasTask = false;

        if (!response.ok) {
            qCWarning(lcQcService) << "fetchTask failed:" << response.error;
            emit errorOccurred(response.error.isEmpty() ? QStringLiteral("fetchTask failed") : response.error);
        } else if (response.data.isNull() || response.data.isUndefined()) {
            qCInfo(lcQcService) << "fetchTask returns null data";
            emit noTaskAvailable();
            m_firstFetch = false;
        } else {
            const QJsonObject taskObject = response.data.toObject();
            if (taskObject.isEmpty()) {
                qCInfo(lcQcService) << "fetchTask returns empty object";
                emit noTaskAvailable();
                m_firstFetch = false;
            } else {
                ImageDistributeInfo info = ImageDistributeInfo::fromJson(taskObject);
                qCInfo(lcQcService) << "fetchTask success, taskId=" << info.id << ", type=" << info.type;
                m_lastTaskId = info.id;
                m_firstFetch = false;
                hasTask = true;
                emit imageDistributeInfoReceived(info);
            }
        }

        if (m_pollingEnabled && !hasTask) {
            m_pollTimer->start();
        }
    });
    watcher->setFuture(m_api->fetchTask(request));
}
