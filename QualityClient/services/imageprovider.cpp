#include "imageprovider.h"

#include <QDateTime>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThreadPool>
#include <QUrl>
namespace {
constexpr int kMaxCacheCostKb = 50 * 1024;  // 50 MB
constexpr qint64 kRetryCooldownMs = 5000;
}  // namespace

ImageProvider& ImageProvider::instance() {
  static ImageProvider provider;
  return provider;
}

ImageProvider::ImageProvider(QObject* parent)
    : QObject(parent),
      m_manager(new QNetworkAccessManager(this)),
      m_cache(kMaxCacheCostKb) {}

void ImageProvider::loadRemote(const QString& url) {
  QNetworkRequest req((QUrl(url)));
  req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);

  QNetworkReply* reply = m_manager->get(req);
  connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
    if (reply->error() != QNetworkReply::NoError) {
      processImageResult(url, QImage(), false, false);
      reply->deleteLater();
      return;
    }

    const QByteArray data = reply->readAll();
    reply->deleteLater();

    // 下载完成后，将二进制解码丢入子线程，不卡 UI
    QThreadPool::globalInstance()->start([this, url, data]() {
      QImage image;
      bool success = image.loadFromData(data);
      processImageResult(url, image, success, false);
    });
  });
}

void ImageProvider::loadLocal(const QString& path) {
  QThreadPool::globalInstance()->start([this, path]() {
    QImage image;
    bool success = image.load(path);
    processImageResult(path, image, success, true);
  });
}

QPixmap ImageProvider::cached(const QString& urlOrPath) const {
  if (urlOrPath.isEmpty()) return QPixmap{};

  QMutexLocker locker(&m_mutex);
  // 去掉 QFileInfo 校验，只做纯内存查找
  if (QPixmap* pix = m_cache.object(urlOrPath)) {
    return *pix;
  }
  return QPixmap{};
}

void ImageProvider::request(const QString& urlOrPath) {
  if (urlOrPath.isEmpty()) return;

  {
    QMutexLocker locker(&m_mutex);
    if (m_inFlight.contains(urlOrPath)) return;

    if (m_cache.contains(urlOrPath)) {
      if (isRemoteUrl(urlOrPath)) return;

      QFileInfo info(urlOrPath);
      if (m_localTimestamps.value(urlOrPath) ==
          info.lastModified().toMSecsSinceEpoch()) {
        return;
      }
    }

    if (isRemoteUrl(urlOrPath)) {
      if (m_nextRetryAtMs.value(urlOrPath, 0) >
          QDateTime::currentMSecsSinceEpoch()) {
        return;
      }
    }
    m_inFlight.insert(urlOrPath);
  }

  if (isRemoteUrl(urlOrPath)) {
    loadRemote(urlOrPath);
  } else {
    loadLocal(urlOrPath);
  }
}

bool ImageProvider::isRemoteUrl(const QString& urlOrPath) const {
  if (urlOrPath.isEmpty()) return false;
  return urlOrPath.startsWith(QLatin1String("http"), Qt::CaseInsensitive) &&
         urlOrPath.contains(QLatin1String("://"));
}

void ImageProvider::processImageResult(const QString& key, const QImage& image,
                                       bool success, bool isLocal) {
  QMetaObject::invokeMethod(
      this,
      [this, key, image, success, isLocal]() {
        if (success && !image.isNull()) {
          // 在主线程将 QImage 转换为 GPU 优化的 QPixmap
          QPixmap* pix = new QPixmap(QPixmap::fromImage(image));

          QMutexLocker locker(&m_mutex);
          m_cache.insert(key, pix, cacheCost(*pix));

          if (isLocal) {
            m_localTimestamps[key] =
                QFileInfo(key).lastModified().toMSecsSinceEpoch();
          } else {
            m_nextRetryAtMs.remove(key);
          }
          m_inFlight.remove(key);
        } else {
          QMutexLocker locker(&m_mutex);
          m_inFlight.remove(key);
          m_cache.remove(key);
          if (!isLocal) {
            m_nextRetryAtMs[key] =
                QDateTime::currentMSecsSinceEpoch() + kRetryCooldownMs;
          }
        }
        emit imageUpdated(key, success);
      },
      Qt::QueuedConnection);
}

void ImageProvider::clearCache(const QString& urlOrPath) {
  QMutexLocker locker(&m_mutex);
  m_cache.remove(urlOrPath);
  m_localTimestamps.remove(urlOrPath);
  m_nextRetryAtMs.remove(urlOrPath);
}

int ImageProvider::cacheCost(const QPixmap& pixmap) const {
  const int bytes =
      qMax(1, pixmap.width() * pixmap.height() * pixmap.depth() / 8);
  return qMax(1, bytes / 1024);
}
