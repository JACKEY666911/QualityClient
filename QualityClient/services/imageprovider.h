#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QCache>
#include <QHash>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QSet>

class QNetworkAccessManager;

class ImageProvider : public QObject {
  Q_OBJECT
 public:
  static ImageProvider& instance();

  QPixmap cached(const QString& urlOrPath) const;
  void request(const QString& urlOrPath);
  bool isRemoteUrl(const QString& urlOrPath) const;

  void processImageResult(const QString& key, const QImage& image, bool success,
                          bool isLocal);
  void clearCache(const QString& urlOrPath);

 signals:
  void imageUpdated(const QString& url, bool success);

 private:
  explicit ImageProvider(QObject* parent = nullptr);
  ~ImageProvider() = default;

  // 具体的加载实现
  void loadRemote(const QString& url);
  void loadLocal(const QString& path);

  int cacheCost(const QPixmap& pixmap) const;

  QNetworkAccessManager* m_manager;
  mutable QCache<QString, QPixmap> m_cache;
  QSet<QString> m_inFlight;
  QHash<QString, qint64> m_nextRetryAtMs;
  QMap<QString, qint64> m_localTimestamps;  // 本地文件最后修改时间校验

  mutable QMutex m_mutex;  // 保护状态位线程安全

  // 禁用拷贝
  ImageProvider(const ImageProvider&) = delete;
  ImageProvider& operator=(const ImageProvider&) = delete;
};

#endif  // IMAGEPROVIDER_H
