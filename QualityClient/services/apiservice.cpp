#include "apiservice.h"
#include "services/settingsservice.h"

ApiService &ApiService::instance()
{
    static ApiService instance;
    return instance;
}

ApiService::ApiService(QObject *parent)
    : QObject(parent)
    , m_http(new HttpClientAsync::HttpClientAsync(this))
{
    {
        SettingsService &settings = SettingsService::instance();
        QString baseUrl = settings.baseUrl();
        if (!baseUrl.isEmpty()) {
            m_http->setBaseUrl(baseUrl);
        }
    }

    m_http->setRequestInterceptor([this](QNetworkRequest &request) {
        if (m_token.isEmpty()) {
            return;
        }
        QString head = m_tokenHead.trimmed();
        if (head.isEmpty()) {
            head = QStringLiteral("Bearer");
        }
        if (!head.endsWith(QLatin1Char(' '))) {
            head += QLatin1Char(' ');
        }
        const QByteArray value = (head + m_token).toUtf8();
        request.setRawHeader("Authorization", value);
    });
}

ApiService::~ApiService() = default;

void ApiService::setBaseUrl(const QString &baseUrl)
{
    m_http->setBaseUrl(baseUrl);
}

void ApiService::setDefaultTimeout(int ms)
{
    m_http->setDefaultTimeout(ms);
}

void ApiService::setDefaultConfig(const HttpClientAsync::RequestConfig &config)
{
    m_http->setDefaultConfig(config);
}

void ApiService::setRequestInterceptor(HttpClientAsync::Interceptor interceptor)
{
    m_http->setRequestInterceptor(std::move(interceptor));
}

void ApiService::setAuthToken(const QString &tokenHead, const QString &token)
{
    m_tokenHead = tokenHead;
    m_token = token;
}

HttpClientAsync::RequestBuilder ApiService::request(const QString &path)
{
    return m_http->request(path);
}

QFuture<HttpClientAsync::HttpResponse> ApiService::login(const LoginRequest &request)
{
    return m_http->request(API_LOGIN).jsonBody(request.toJson()).post();
}

QFuture<HttpClientAsync::HttpResponse> ApiService::fetchTask(const ImageDistributeRequest &request)
{
    return m_http->request(API_IAMGE_DISTRIBUTE).jsonBody(request.toJson()).post();
}


#include <QStringLiteral>
const QString ApiService::API_LOGIN = QStringLiteral("/web/device/login");
const QString ApiService::API_LOG_OUT = QStringLiteral("/web/device/logout");
const QString ApiService::API_USER_INFO = QStringLiteral("/web/admin/info");
const QString ApiService::API_SAVEHISTORYRECORD = QStringLiteral("/qc/task/count/latest/save");
const QString ApiService::API_QUERYPERSONBAGGAGE = QStringLiteral("/pb/relate/person-baggage");
const QString ApiService::API_MANUALXRAY = QStringLiteral("/pb/relate/manual/bind/xray");
const QString ApiService::API_IAMGE_DISTRIBUTE = QStringLiteral("/qc/task/distribute");
const QString ApiService::API_IAMGE_DISTRIBUTE2 = QStringLiteral("/qc/task/distribute/");
const QString ApiService::API_ADDEXTRACTION_TEMPTASK = QStringLiteral("/qc/source/extraction/distribute/temp-task/add");
const QString ApiService::API_ADD_REEXTARCT_TASK = QStringLiteral("/qc/source/extraction/distribute/re-extract-task/add");
const QString ApiService::API_SAVE_QUALITY_INFOS = QStringLiteral("/qc/task/result/save");
const QString ApiService::API_SAVE_CONTRABAND_BAG_IMAGE = QStringLiteral("/qc/task/xray4web/save");
const QString ApiService::API_QUERY_HISTORYR_IMAGE = QStringLiteral("/qc/task/already/detail/query/");
const QString ApiService::API_UPDATE_QUALITY_RESULT = QStringLiteral("/qc/task/result/update");
const QString ApiService::API_QUERY_HISTORYR_TIMELIST = QStringLiteral("/qc/task/already/page/query");
const QString ApiService::API_QUERY_HISTORYR_TIMELIST_OLD = QStringLiteral("/qc/task/already/list/query");
const QString ApiService::API_SAVE_CONTRABAND = QStringLiteral("/qc/task/tag/save");
const QString ApiService::API_TASK_FINISH = QStringLiteral("/qc/task/finish");
const QString ApiService::API_OPENBAG_RES = QStringLiteral("/ob/openbaggage/open-result");
const QString ApiService::API_UPDATE_CONTRABAND = QStringLiteral("/qc/task/tag/update");
const QString ApiService::API_QUERY_IMAGE_DESCRIPTION = QStringLiteral("/nr/qc/enhanced/type/query");
const QString ApiService::API_GET_NVR_INFOS = QStringLiteral("/qc/resource/nvr/query");
const QString ApiService::API_MODE_SELECT = QStringLiteral("/qc/task/distribute/mode");
const QString ApiService::API_MODE_SELECT_FINISH = QStringLiteral("/qc/task/finish/mode");
const QString ApiService::API_CLIENTS_HEARTBEAT = QStringLiteral("/qc/client/heartbeat");
const QString ApiService::API_GET_AREA_LIST = QStringLiteral("/web/area/treeList");
const QString ApiService::API_GET_USER_LIST = QStringLiteral("/qc/task/already/user/query");
const QString ApiService::API_NEW_QUERY_DETAIL_HISTORY = QStringLiteral("/qc/task/already/detail/query/v2/");
