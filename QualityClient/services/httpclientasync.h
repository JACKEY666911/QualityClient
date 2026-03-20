#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFuture>
#include <QPromise>
#include <QException>
#include <QPointer>
#include <functional>
namespace HttpClientAsync {

// 响应结构
struct HttpResponse {
    int status = -1;
    QByteArray body;
    QJsonDocument json;
    QJsonValue data;
    bool ok = false;
    QString error;
    bool isTimeout = false;
    // 便捷方法

    QJsonObject dataObject() const { return data.toObject(); }
    QJsonArray dataArray() const { return data.toArray(); }

    QJsonObject object() const { return json.object(); }
    QJsonArray array() const { return json.array(); }
    template<typename T>
    T get(const QString& key) const { return object().value(key).toVariant().value<T>(); }
};

// HTTP 异常
class HttpException : public QException {
public:
    explicit HttpException(int code, QString msg)
        : m_code(code), m_message(std::move(msg)) {}

    void raise() const override { throw *this; }
    QException* clone() const override { return new HttpException(*this); }

    int code() const { return m_code; }
    const QString& message() const { return m_message; }

private:
    int m_code;
    QString m_message;
};

// 请求配置
struct RequestConfig {
    int timeoutMs = 30000;
    int maxRetries = 0;           // 重试次数
    int retryDelayMs = 1000;      // 基础延迟
    bool jitter = true;            // 抖动，避免重试风暴
    float jitterFactor = 0.5f;   // 抖动范围 0.5-1.5 倍
};

using Interceptor = std::function<void(QNetworkRequest&)>;

// 前向声明
class HttpClientAsync;

// 请求构建器
class RequestBuilder {
public:
    RequestBuilder(HttpClientAsync* client, QString path);

    // 链式配置
    RequestBuilder& query(const QString& key, const QString& value);
    RequestBuilder& header(const QByteArray& key, const QByteArray& value);
    RequestBuilder& jsonBody(const QJsonDocument& doc);
    RequestBuilder& jsonBody(const QJsonObject& obj);
    RequestBuilder& jsonBody(const QJsonArray& arr);
    RequestBuilder& body(const QByteArray& data, const QString& contentType);
    RequestBuilder& config(const RequestConfig& cfg);

    // 执行方法（返回 QFuture）
    QFuture<HttpResponse> get();
    QFuture<HttpResponse> post();
    QFuture<HttpResponse> put();
    QFuture<HttpResponse> del();
    QFuture<HttpResponse> patch();

    // 便捷：直接解析为类型
    template<typename T>
    QFuture<T> getAs(std::function<T(const HttpResponse&)> parser);

private:
    HttpClientAsync* m_client;
    QString m_path;
    QUrlQuery m_query;
    QMap<QByteArray, QByteArray> m_headers;
    QByteArray m_body;
    RequestConfig m_config;
    bool m_hasBody = false;
};


// HTTP 客户端
class HttpClientAsync : public QObject {
    Q_OBJECT
public:
    explicit HttpClientAsync(QObject *parent = nullptr);
    ~HttpClientAsync();

    // 配置
    void setBaseUrl(const QString& baseUrl);
    void setDefaultTimeout(int ms);
    void setDefaultConfig(const RequestConfig& config);
    void setRequestInterceptor(Interceptor interceptor);

    // 创建请求
    RequestBuilder request(const QString& path);

    // 底层执行（内部使用）
    QFuture<HttpResponse> execute(const QString& path,
                                  const QUrlQuery& query,
                                  const QMap<QByteArray, QByteArray>& headers,
                                  const QByteArray& body,
                                  const QByteArray& method,
                                  const RequestConfig& config);

private:
    HttpResponse makeResponse(QNetworkReply* reply, const QByteArray& bytes);
    int calculateDelay(int baseDelay, bool jitter, float jitterFactor);
    QFuture<HttpResponse> doRequest(const QNetworkRequest& req,
                                    const QByteArray& method,
                                    const QByteArray& body,
                                    const RequestConfig& config,
                                    int attempt = 0);

    QNetworkAccessManager* m_mgr;
    QString m_baseUrl;
    RequestConfig m_defaultConfig;
    Interceptor m_interceptor;
    friend class RequestBuilder;
};

}