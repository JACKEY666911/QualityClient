#pragma once

#include <QObject>

#include "httpclientasync.h"
#include "Models/ApiRequests.h"

class ApiService : public QObject
{
    Q_OBJECT
public:
    static ApiService &instance();

    explicit ApiService(QObject *parent = nullptr);
    ~ApiService() override;

    void setBaseUrl(const QString &baseUrl);
    void setDefaultTimeout(int ms);
    void setDefaultConfig(const HttpClientAsync::RequestConfig &config);
    void setRequestInterceptor(HttpClientAsync::Interceptor interceptor);
    void setAuthToken(const QString &tokenHead, const QString &token);

    HttpClientAsync::RequestBuilder request(const QString &path);

    QFuture<HttpClientAsync::HttpResponse> login(const LoginRequest &request);
    QFuture<HttpClientAsync::HttpResponse> fetchTask(const ImageDistributeRequest &request);

    static const QString API_LOGIN;
    static const QString API_LOG_OUT;
    static const QString API_USER_INFO;
    static const QString API_SAVEHISTORYRECORD;
    static const QString API_QUERYPERSONBAGGAGE;
    static const QString API_MANUALXRAY;
    static const QString API_IAMGE_DISTRIBUTE;
    static const QString API_IAMGE_DISTRIBUTE2;
    static const QString API_ADDEXTRACTION_TEMPTASK;
    static const QString API_ADD_REEXTARCT_TASK;
    static const QString API_SAVE_QUALITY_INFOS;
    static const QString API_SAVE_CONTRABAND_BAG_IMAGE;
    static const QString API_QUERY_HISTORYR_IMAGE;
    static const QString API_UPDATE_QUALITY_RESULT;
    static const QString API_QUERY_HISTORYR_TIMELIST;
    static const QString API_QUERY_HISTORYR_TIMELIST_OLD;
    static const QString API_SAVE_CONTRABAND;
    static const QString API_TASK_FINISH;
    static const QString API_OPENBAG_RES;
    static const QString API_UPDATE_CONTRABAND;
    static const QString API_QUERY_IMAGE_DESCRIPTION;
    static const QString API_GET_NVR_INFOS;
    static const QString API_MODE_SELECT;
    static const QString API_MODE_SELECT_FINISH;
    static const QString API_CLIENTS_HEARTBEAT;
    static const QString API_GET_AREA_LIST;
    static const QString API_GET_USER_LIST;
    static const QString API_NEW_QUERY_DETAIL_HISTORY;

private:
    HttpClientAsync::HttpClientAsync *m_http;
    QString m_tokenHead;
    QString m_token;
};
