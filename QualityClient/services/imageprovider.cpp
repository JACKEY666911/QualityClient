#include "imageprovider.h"
#include "logging/logcategories.h"

#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace {
constexpr int kMaxCacheCostKb = 50 * 1024; // 50 MB
constexpr qint64 kRetryCooldownMs = 5000;
}

ImageProvider &ImageProvider::instance()
{
    static ImageProvider provider;
    return provider;
}

ImageProvider::ImageProvider(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_cache(kMaxCacheCostKb)
{
}

QPixmap ImageProvider::cached(const QString &url) const
{
    if (url.isEmpty()) {
        return QPixmap();
    }

    if (QPixmap *pix = m_cache.object(url)) {
        return *pix;
    }

    if (!isRemoteUrl(url)) {
        QPixmap local(url);
        if (!local.isNull()) {
            m_cache.insert(url, new QPixmap(local), cacheCost(local));
        }
        return local;
    }

    return QPixmap();
}

void ImageProvider::request(const QString &url)
{
    if (!isRemoteUrl(url) || m_inFlight.contains(url) || m_cache.contains(url)) {
        return;
    }

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    if (m_nextRetryAtMs.value(url, 0) > nowMs) {
        qCDebug(lcQcService) << "[ImageProvider] url in cooldown, skip request:" << url;
        return;
    }

    m_inFlight.insert(url);
    qCDebug(lcQcService) << "[ImageProvider] request start:" << url;
    QNetworkReply *reply = m_manager->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
        m_inFlight.remove(url);

        bool success = false;
        if (reply->error() == QNetworkReply::NoError) {
            QPixmap pixmap;
            if (pixmap.loadFromData(reply->readAll()) && !pixmap.isNull()) {
                m_cache.insert(url, new QPixmap(pixmap), cacheCost(pixmap));
                m_nextRetryAtMs.remove(url);
                success = true;
            }
        }

        if (!success) {
            m_cache.remove(url);
            m_nextRetryAtMs.insert(url, QDateTime::currentMSecsSinceEpoch() + kRetryCooldownMs);
            qCWarning(lcQcService) << "[ImageProvider] request failed:" << url
                                   << "err=" << reply->errorString();
        } else {
            qCDebug(lcQcService) << "[ImageProvider] request success:" << url;
        }

        emit imageUpdated(url, success);
        reply->deleteLater();
    });
}

bool ImageProvider::isRemoteUrl(const QString &url) const
{
    const QUrl parsed(url);
    const QString scheme = parsed.scheme().toLower();
    return parsed.isValid() && (scheme == QStringLiteral("http") || scheme == QStringLiteral("https"));
}

int ImageProvider::cacheCost(const QPixmap &pixmap) const
{
    const int bytes = qMax(1, pixmap.width() * pixmap.height() * pixmap.depth() / 8);
    return qMax(1, bytes / 1024);
}
