#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QObject>
#include <QCache>
#include <QHash>
#include <QPixmap>
#include <QSet>

class QNetworkAccessManager;

class ImageProvider : public QObject
{
    Q_OBJECT
public:
    static ImageProvider &instance();

    QPixmap cached(const QString &url) const;
    void request(const QString &url);

signals:
    void imageUpdated(const QString &url, bool success);

private:
    explicit ImageProvider(QObject *parent = nullptr);
    bool isRemoteUrl(const QString &url) const;
    int cacheCost(const QPixmap &pixmap) const;

    QNetworkAccessManager *m_manager;
    mutable QCache<QString, QPixmap> m_cache;
    QSet<QString> m_inFlight;
    QHash<QString, qint64> m_nextRetryAtMs;
};

#endif // IMAGEPROVIDER_H
