#include "httpclientasync.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QUrl>
namespace HttpClientAsync {
// 构造函数：保存客户端指针和路径，复制默认配置
RequestBuilder::RequestBuilder(HttpClientAsync* client, QString path)
    : m_client(client)                    // 保存 HttpClientAsync 指针
    , m_path(std::move(path))              // 移动语义，避免字符串拷贝
    , m_config(client->m_defaultConfig)    // 复制客户端的默认配置
    , m_hasBody(false)                     // 初始无请求体
{}

// 链式添加查询参数 ?key=value
RequestBuilder& RequestBuilder::query(const QString& key, const QString& value) {
    m_query.addQueryItem(key, value);       // QUrlQuery 添加参数
    return *this;                           // 返回自身引用，支持链式调用
}

// 链式添加 HTTP 头
RequestBuilder& RequestBuilder::header(const QByteArray& key, const QByteArray& value) {
    m_headers[key] = value;               // QMap 插入
    return *this;                           // 返回自身引用
}

// 设置 JSON 请求体（QJsonDocument 版本）
RequestBuilder& RequestBuilder::jsonBody(const QJsonDocument& doc) {
    m_body = doc.toJson(QJsonDocument::Compact);     // 压缩格式 JSON
    m_headers["Content-Type"] = "application/json"; // 自动设置 Content-Type
    m_hasBody = true;                                // 标记有请求体
    return *this;
}

// 便捷重载：QJsonObject → QJsonDocument
RequestBuilder& RequestBuilder::jsonBody(const QJsonObject& obj) {
    return jsonBody(QJsonDocument(obj));      // 委托给上面的实现
}

// 便捷重载：QJsonArray → QJsonDocument
RequestBuilder& RequestBuilder::jsonBody(const QJsonArray& arr) {
    return jsonBody(QJsonDocument(arr));      // 委托给上面的实现
}

// 设置原始字节请求体
RequestBuilder& RequestBuilder::body(const QByteArray& data, const QString& contentType) {
    m_body = data;                                    // 保存原始数据
    m_headers["Content-Type"] = contentType.toUtf8(); // 设置 Content-Type
    m_hasBody = true;
    return *this;
}

// 设置本次请求的专属配置
RequestBuilder& RequestBuilder::config(const RequestConfig& cfg) {
    m_config = cfg;      // 整体替换配置
    return *this;
}

// GET 请求：调用客户端执行
QFuture<HttpResponse> RequestBuilder::get() {
    // 空 body，"GET" 方法
    return m_client->execute(m_path, m_query, m_headers,
                             QByteArray(), "GET", m_config);
}

// POST 请求
QFuture<HttpResponse> RequestBuilder::post() {
    return m_client->execute(m_path, m_query, m_headers,
                             m_body, "POST", m_config);
}

// PUT 请求
QFuture<HttpResponse> RequestBuilder::put() {
    return m_client->execute(m_path, m_query, m_headers,
                             m_body, "PUT", m_config);
}

// DELETE 请求
QFuture<HttpResponse> RequestBuilder::del() {
    return m_client->execute(m_path, m_query, m_headers,
                             QByteArray(), "DELETE", m_config);
}

// PATCH 请求
QFuture<HttpResponse> RequestBuilder::patch() {
    return m_client->execute(m_path, m_query, m_headers,
                             m_body, "PATCH", m_config);
}

// ========== HttpClientAsync 实现 ==========
HttpClientAsync::HttpClientAsync(QObject *parent)
    : QObject{parent}, m_mgr{new QNetworkAccessManager(this)},
    m_baseUrl{},                                       // 空基础 URL
    m_defaultConfig{},                                   // 默认配置（30秒超时）
    m_interceptor(nullptr)                              // 无拦截器
{

}
// 析构函数
HttpClientAsync::~HttpClientAsync() = default;

// 设置基础 URL（处理尾部斜杠）
void HttpClientAsync::setBaseUrl(const QString& baseUrl) {
    m_baseUrl = baseUrl;
    // 确保不以 / 结尾，避免 https://api.com//path 双斜杠问题
    if (m_baseUrl.endsWith('/')) {
        m_baseUrl.chop(1);      // 删除最后一个字符
    }
}

// 设置默认超时
void HttpClientAsync::setDefaultTimeout(int ms) {
    m_defaultConfig.timeoutMs = ms;
}

// 设置默认配置
void HttpClientAsync::setDefaultConfig(const RequestConfig& config) {
    m_defaultConfig = config;
}

// 设置请求拦截器（移动语义，转移所有权）
void HttpClientAsync::setRequestInterceptor(Interceptor interceptor) {
    m_interceptor = std::move(interceptor);
}

// 工厂方法：创建请求构建器
RequestBuilder HttpClientAsync::request(const QString& path) {
    return RequestBuilder(this, path);      // 返回临时对象，支持链式
}

// ========== 核心：执行 HTTP 请求 ==========

QFuture<HttpResponse> HttpClientAsync::execute(const QString& path,
                                               const QUrlQuery& query,
                                               const QMap<QByteArray, QByteArray>& headers,
                                               const QByteArray& body,
                                               const QByteArray& method,
                                               const RequestConfig& config)
{
    // 1. 构建完整 URL
    QUrl url(m_baseUrl + path);     // 基础 URL + 路径
    url.setQuery(query);             // 添加查询参数 ?page=1&limit=20

    QNetworkRequest req(url);        // 创建请求对象

    // 2. 应用全局拦截器（如注入 Token）
    if (m_interceptor) {             // 检查是否有拦截器
        m_interceptor(req);            // 调用拦截器修改请求
    }

    // 3. 应用本次请求的特定 headers
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        req.setRawHeader(it.key(), it.value());      // 设置原始字节头
    }

    // 4. 委托给实际执行方法
    return doRequest(req, method, body, config, 0);   // 0 = 第0次尝试（首次）
}

// ========== 核心：实际发送请求（QPromise 关键）==========

QFuture<HttpResponse> HttpClientAsync::doRequest(const QNetworkRequest& req,
                                                 const QByteArray& method,
                                                 const QByteArray& body,
                                                 const RequestConfig& config,
                                                 int attempt)
{
    Q_UNUSED(attempt)
    //使用 shared_ptr 管理 QPromise，解决复制问题
    auto promise = std::make_shared<QPromise<HttpResponse>>();
    QFuture<HttpResponse> future = promise->future();
    promise->start();

    QNetworkReply* reply = m_mgr->sendCustomRequest(req, method, body);

    QTimer* timer = new QTimer(reply);
    timer->setSingleShot(true);
    // 超时处理
    connect(timer, &QTimer::timeout, this, [reply]() {
        if (reply && !reply->isFinished()) { // 防重复abort
            reply->abort();
        }
    });

    //关键修复：捕获 shared_ptr 而非原始 promise
    connect(reply, &QNetworkReply::finished, this,
            [this, promise, reply, timer]() {

                timer->stop();
                // 检查是否取消
                if (promise->isCanceled()) {
                    reply->deleteLater();
                    promise->finish(); // 标记Promise完成
                    return;
                }
                QByteArray bytes = reply->readAll();
                HttpResponse resp = makeResponse(reply, bytes);

                // 设置最终结果
                // if (resp.ok) {
                //     promise->addResult(std::move(resp));
                // } else {
                //     promise->setException(
                //         HttpException(resp.status, resp.error)
                //         );
                // }
                if (!promise->isCanceled()) {
                    promise->addResult(std::move(resp));
                }
                promise->finish();
                reply->deleteLater();
            });

    timer->start(config.timeoutMs);
    return future;
}

// ========== 构造响应对象 ==========

HttpResponse HttpClientAsync::makeResponse(QNetworkReply* reply,
                                           const QByteArray& bytes)
{
    HttpResponse r;
    r.body = bytes;

    if (reply->error() != QNetworkReply::NoError) {
        r.ok = false;
        r.isTimeout = (reply->error() == QNetworkReply::OperationCanceledError);
        r.error = reply->errorString();
        // 尝试记录物理状态码供调试
        r.status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        return r;
    }

    if (!bytes.isEmpty()) {
        QJsonParseError parseErr;
        r.json = QJsonDocument::fromJson(bytes, &parseErr);

        if (parseErr.error == QJsonParseError::NoError) {
            QJsonObject root = r.json.object();

            // 提取固定结构：status, message, data
            if (root.contains(QStringLiteral("status"))) {
                // 1. 提取业务状态码
                r.status = root.value(QStringLiteral("status")).toVariant().toInt();
                // 2. 提取业务消息
                r.error = root.value(QStringLiteral("message")).toString();
                // 3. 提取业务核心数据
                r.data = root.value(QStringLiteral("data"));

                //只有 status == 200 代表接口正常业务返回
                r.ok = (r.status == 200);
            } else {
                // JSON 结构不对（没有 status 字段）
                r.ok = false;
                r.error = QStringLiteral("非法业务报文格式");
            }
        } else {
            // 返回的不是 JSON
            r.ok = false;
            r.error = QStringLiteral("无法解析服务器响应数据");
        }
    } else {
        // 空响应
        r.ok = false;
        r.error = QStringLiteral("服务器返回空报文");
    }
    return r;
}

int HttpClientAsync::calculateDelay(int baseDelay, bool jitter, float jitterFactor)
{
    if (!jitter) return baseDelay;

    // 指数退避 + 随机抖动
    float minFactor = 1.0f - jitterFactor;
    float maxFactor = 1.0f + jitterFactor;
    float random = QRandomGenerator::global()->generateDouble();
    float factor = minFactor + random * (maxFactor - minFactor);

    return static_cast<int>(baseDelay * factor);
}

}
