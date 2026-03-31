#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QWidget>

class HikvisionSdkService : public QObject {
    Q_OBJECT
public:
    enum PlaybackSlot { FrontSlot = 0, BackSlot = 1 };

    struct PlaybackFile {
        QString fileName;
        QDateTime startTime;
        QDateTime endTime;
        quint32 fileSize = 0;
    };

    static HikvisionSdkService &instance();

    explicit HikvisionSdkService(QObject *parent = nullptr);
    ~HikvisionSdkService() override;

    bool initialize(const QString &sdkDir = QString());
    void shutdown();

    bool login(const QString &ip, int port, const QString &user, const QString &password);
    void logout();

    bool startPreview(WId winId, int channel, int streamType = 0, int linkMode = 0, bool blocked = true);
    bool stopPreview();

    bool startPlaybackByTime(int slot, WId winId, int channel, const QDateTime &start,
                             const QDateTime &end);
    bool stopPlayback(int slot);
    bool stopPlaybackAll();
    bool seekPlaybackTime(int slot, const QDateTime &time);
    bool seekPlaybackTimeAll(const QDateTime &time);
    bool pausePlayback(int slot, bool pause);
    bool pausePlaybackAll(bool pause);
    bool playbackOsdTime(int slot, QDateTime &time) const;
    bool setPlaybackSpeed(int slot, double speed);
    bool setPlaybackSpeedAll(double speed);
    bool hasPlayback(int slot) const;
    bool hasAnyPlayback() const;
    QList<PlaybackFile> findFilesByChannel(int channel, const QDateTime &start,
                                           const QDateTime &end, int maxFiles = 200,
                                           bool quickSearch = true);
    QList<PlaybackFile> findFilesByStreamId(const QByteArray &streamId,
                                            const QDateTime &start, const QDateTime &end,
                                            int maxFiles = 200);

    int lastErrorCode() const;
    QString lastErrorMessage() const;

    // Delete copy constructor and assignment operator
    HikvisionSdkService(const HikvisionSdkService &) = delete;
    HikvisionSdkService &operator=(const HikvisionSdkService &) = delete;

private:
    void setLastError(const QString &action);
    bool ensureInitialized();
    static void fillHkTime(const QDateTime &time, void *hkTime);
    static QDateTime fromHkTime(const void *hkTime);
    static bool isValidSlot(int slot);

    bool m_initialized = false;
    long m_userId = -1;
    long m_previewHandle = -1;
    long m_playbackHandles[2] = { -1, -1 };
    int m_lastErrorCode = 0;
    QString m_lastErrorMessage;
};
