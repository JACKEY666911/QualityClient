#include "httpclient.h"
#include <QDebug>
RequestBuilder::RequestBuilder(HttpClient* client, QString path)
    : m_client(client), m_path(std::move(path)) {}

RequestBuilder& RequestBuilder::query(const QString& key, const QString& value) {
    m_query.addQueryItem(key, value);
    return *this;
}

RequestBuilder& RequestBuilder::header(const QByteArray& key, const QByteArray& value) {
    m_headers[key] = value;
    return *this;
}

RequestBuilder& RequestBuilder::jsonBody(const QJsonDocument& doc) {
    m_body = doc.toJson(QJsonDocument::Compact);
    m_headers["Content-Type"] = "application/json";
    m_hasBody = true;
    return *this;
}

RequestBuilder& RequestBuilder::jsonBody(const QJsonObject& obj) {
    return jsonBody(QJsonDocument(obj));
}

RequestBuilder& RequestBuilder::jsonBody(const QJsonArray& arr) {
    return jsonBody(QJsonDocument(arr));
}

void RequestBuilder::get(std::function<void(const HttpResponse&)> cb) {
    m_client->execute(m_path, m_query, m_headers, QByteArray(), "GET", cb);
}

void RequestBuilder::post(std::function<void(const HttpResponse&)> cb) {
    m_client->execute(m_path, m_query, m_headers, m_body, "POST", cb);
}

void RequestBuilder::put(std::function<void(const HttpResponse&)> cb) {
    m_client->execute(m_path, m_query, m_headers, m_body, "PUT", cb);
}

void RequestBuilder::del(std::function<void(const HttpResponse&)> cb) {
    m_client->execute(m_path, m_query, m_headers, QByteArray(), "DELETE", cb);
}


// --- HttpClient 实现 ---

HttpClient::HttpClient(QObject* parent)
    : QObject(parent), m_mgr(new QNetworkAccessManager(this)) {
}

HttpClient::~HttpClient() {
}

void HttpClient::setBaseUrl(const QString& baseUrl) {
    m_baseUrl = baseUrl;
    // 确保 BaseUrl 不以 '/' 结尾，避免双斜杠问题
    if (m_baseUrl.endsWith('/')) {
        m_baseUrl.chop(1);
    }
}

void HttpClient::setTimeoutMs(int ms) {
    m_timeoutMs = ms;
}

void HttpClient::setRequestInterceptor(Interceptor interceptor) {
    m_interceptor = std::move(interceptor);
}

RequestBuilder HttpClient::request(const QString& path) {
    return RequestBuilder(this, path);
}

void HttpClient::execute(const QString& path,
                         const QUrlQuery& query,
                         const QMap<QByteArray, QByteArray>& headers,
                         const QByteArray& body,
                         const QByteArray& method,
                         std::function<void(const HttpResponse&)> cb)
{
    // 1. 构建 URL
    QUrl url(m_baseUrl + path);
    url.setQuery(query);

    QNetworkRequest req(url);

    // 2. 设置全局拦截器 (例如注入 Token)
    if (m_interceptor) {
        m_interceptor(req);
    }

    // 3. 设置本次请求特定的 Header
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        req.setRawHeader(it.key(), it.value());
    }
    qDebug() << "1111111111111111111111111111111";
    // 4. 发送请求
    QNetworkReply* reply = m_mgr->sendCustomRequest(req, method, body);

    // 5. 设置超时定时器
    QTimer* timer = new QTimer(reply);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, reply, [reply]() {
        qDebug() << "2222222222222222222222222222";
        reply->abort(); // 超时中止
    });
    timer->start(m_timeoutMs);

    // 6. 处理响应
    connect(reply, &QNetworkReply::finished, this, [this, reply, cb, timer]() mutable {
        timer->stop();

        // 读取数据
        QByteArray bytes = reply->readAll();
        HttpResponse resp = makeResponse(reply, bytes);

        // 回调必须在 reply delete 之前执行，因为回调中可能需要读取 reply 的某些属性
        if (cb) {
            cb(resp);
        }

        // 清理资源
        reply->deleteLater(); // timer 会作为子对象自动销毁
    });
}

HttpResponse HttpClient::makeResponse(QNetworkReply* reply, const QByteArray& bytes) {
    HttpResponse r;
    r.bytes = bytes;

    // 获取 HTTP 状态码
    QVariant statusVar = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    r.status = statusVar.isValid() ? statusVar.toInt() : -1;

    // 处理网络层错误
    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            r.error = "Request Timeout";
        } else {
            r.error = reply->errorString();
        }
        r.ok = false;
        return r;
    }

    // 尝试解析 JSON (仅当内容非空时)
    if (!bytes.isEmpty()) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
        if (err.error == QJsonParseError::NoError) {
            r.json = doc;
        } else {
            // 如果不是 JSON，不报错，r.json 保持空，r.bytes 有数据
            // r.error = QString("JSON parse error: %1").arg(err.errorString());
        }
    }

    // 判断 HTTP 状态码是否正常
    r.ok = (r.status >= 200 && r.status < 300);
    return r;
}
