#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

// 响应结构体
struct HttpResponse {
    int status = -1;              // HTTP 状态码
    QByteArray bytes;             // 原始字节数据
    QJsonDocument json;           // 解析后的 JSON (自动尝试解析)
    QString error;                // 错误描述 (网络错误或解析错误)
    bool ok = false;              // 是否成功 (网络层成功 + HTTP 2xx)

    // 辅助方法：判断是否有有效的 JSON 对象
    bool isJson() const { return !json.isNull(); }

    // 辅助方法：获取 JSON 对象
    QJsonObject toObject() const { return json.object(); }

    // 辅助方法：获取 JSON 数组
    QJsonArray toArray() const { return json.array(); }
};

class HttpClient;

// 请求构建器 (实现链式调用)
class RequestBuilder {
public:
    RequestBuilder(HttpClient* client, QString path);

    // 设置查询参数
    RequestBuilder& query(const QString& key, const QString& value);

    // 设置请求头
    RequestBuilder& header(const QByteArray& key, const QByteArray& value);

    // 设置 JSON Body (QJsonDocument)
    RequestBuilder& jsonBody(const QJsonDocument& doc);

    // 设置 JSON Body (QJsonObject)
    RequestBuilder& jsonBody(const QJsonObject& obj);

    // 设置 JSON Body (QJsonArray)
    RequestBuilder& jsonBody(const QJsonArray& arr);

    // 发起请求
    void get(std::function<void(const HttpResponse&)> cb);
    void post(std::function<void(const HttpResponse&)> cb);
    void put(std::function<void(const HttpResponse&)> cb);
    void del(std::function<void(const HttpResponse&)> cb);

private:
    HttpClient* m_client;
    QString m_path;
    QUrlQuery m_query;
    QMap<QByteArray, QByteArray> m_headers;
    QByteArray m_body;
    bool m_hasBody = false;
};


// HTTP 客户端核心
class HttpClient : public QObject {
    Q_OBJECT
    friend class RequestBuilder;

public:
    explicit HttpClient(QObject* parent = nullptr);
    ~HttpClient();

    // 全局配置
    void setBaseUrl(const QString& baseUrl);
    void setTimeoutMs(int ms);

    // 设置请求拦截器 (例如全局添加 Token)
    using Interceptor = std::function<void(QNetworkRequest&)>;
    void setRequestInterceptor(Interceptor interceptor);

    // 入口：创建请求构建器
    RequestBuilder request(const QString& path);

private:
    void execute(const QString& path,
                 const QUrlQuery& query,
                 const QMap<QByteArray, QByteArray>& headers,
                 const QByteArray& body,
                 const QByteArray& method,
                 std::function<void(const HttpResponse&)> cb);

    HttpResponse makeResponse(QNetworkReply* reply, const QByteArray& bytes);

    QNetworkAccessManager* m_mgr;
    QString m_baseUrl;
    int m_timeoutMs = 15000;
    Interceptor m_interceptor;
};
