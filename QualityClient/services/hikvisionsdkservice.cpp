#include "hikvisionsdkservice.h"

#include <QCoreApplication>
#include <QDir>
#include <QLoggingCategory>

#include <Windows.h>

#include "logging/logcategories.h"
#include "thirdparty/hikvision/include/HCNetSDK.h"

HikvisionSdkService &HikvisionSdkService::instance()
{
    static HikvisionSdkService instance;
    return instance;
}

HikvisionSdkService::HikvisionSdkService(QObject *parent)
    : QObject(parent)
{
}

HikvisionSdkService::~HikvisionSdkService()
{
    shutdown();
}

bool HikvisionSdkService::initialize(const QString &sdkDir)
{
    if (m_initialized) {
        return true;
    }

    if (!NET_DVR_Init()) {
        setLastError(QStringLiteral("NET_DVR_Init"));
        qCWarning(lcHikvisionSdk) << "SDK init failed:" << m_lastErrorMessage;
        return false;
    }

    QString sdkPath = sdkDir;
    if (sdkPath.isEmpty()) {
        sdkPath = QDir::cleanPath(
            QCoreApplication::applicationDirPath() + QStringLiteral("/hikvision"));
    }

    const QByteArray pathBytes = QDir::toNativeSeparators(sdkPath).toLocal8Bit();
    NET_DVR_SetSDKInitCfg(NET_SDK_INIT_CFG_SDK_PATH, (void *)pathBytes.constData());
    NET_DVR_SetSDKInitCfg(NET_SDK_INIT_CFG_LIBEAY_PATH, (void *)pathBytes.constData());
    NET_DVR_SetSDKInitCfg(NET_SDK_INIT_CFG_SSLEAY_PATH, (void *)pathBytes.constData());
    qCInfo(lcHikvisionSdk) << "SDK initialized with path" << sdkPath;

    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(2000, true);

    m_initialized = true;
    m_lastErrorCode = 0;
    m_lastErrorMessage.clear();
    return true;
}

void HikvisionSdkService::shutdown()
{
    if (!m_initialized) {
        return;
    }

    stopPreview();
    stopPlaybackAll();
    logout();

    NET_DVR_Cleanup();
    m_initialized = false;
    qCInfo(lcHikvisionSdk) << "SDK cleanup complete";
}

bool HikvisionSdkService::login(const QString &ip, int port, const QString &user, const QString &password)
{
    if (!ensureInitialized()) {
        return false;
    }

    if (m_userId >= 0) {
        return true;
    }

    NET_DVR_USER_LOGIN_INFO loginInfo = {};
    NET_DVR_DEVICEINFO_V40 deviceInfo = {};
    loginInfo.bUseAsynLogin = 0;
    loginInfo.wPort = static_cast<WORD>(port);
    QByteArray ipBytes = ip.toLocal8Bit();
    QByteArray userBytes = user.toLocal8Bit();
    QByteArray passBytes = password.toLocal8Bit();
    strncpy_s(reinterpret_cast<char *>(loginInfo.sDeviceAddress),
              sizeof(loginInfo.sDeviceAddress), ipBytes.constData(), _TRUNCATE);
    strncpy_s(reinterpret_cast<char *>(loginInfo.sUserName), sizeof(loginInfo.sUserName),
              userBytes.constData(), _TRUNCATE);
    strncpy_s(reinterpret_cast<char *>(loginInfo.sPassword), sizeof(loginInfo.sPassword),
              passBytes.constData(), _TRUNCATE);

    m_userId = NET_DVR_Login_V40(&loginInfo, &deviceInfo);
    if (m_userId < 0) {
        setLastError(QStringLiteral("NET_DVR_Login_V40"));
        qCWarning(lcHikvisionSdk) << "Login failed:" << m_lastErrorMessage;
        return false;
    }

    qCInfo(lcHikvisionSdk) << "Login success, userId=" << m_userId;
    return true;
}

void HikvisionSdkService::logout()
{
    if (m_userId < 0) {
        return;
    }

    NET_DVR_Logout(m_userId);
    m_userId = -1;
    qCInfo(lcHikvisionSdk) << "Logout success";
}

bool HikvisionSdkService::startPreview(WId winId, int channel, int streamType, int linkMode, bool blocked)
{
    if (!ensureInitialized()) {
        return false;
    }
    if (m_userId < 0) {
        setLastError(QStringLiteral("NET_DVR_Login_V30"));
        return false;
    }

    stopPreview();

    NET_DVR_PREVIEWINFO previewInfo = {};
    previewInfo.hPlayWnd = reinterpret_cast<HWND>(winId);
    previewInfo.lChannel = channel;
    previewInfo.dwStreamType = static_cast<DWORD>(streamType);
    previewInfo.dwLinkMode = static_cast<DWORD>(linkMode);
    previewInfo.bBlocked = blocked ? TRUE : FALSE;

    m_previewHandle = NET_DVR_RealPlay_V40(m_userId, &previewInfo, nullptr, nullptr);
    if (m_previewHandle < 0) {
        setLastError(QStringLiteral("NET_DVR_RealPlay_V40"));
        qCWarning(lcHikvisionSdk) << "Start preview failed:" << m_lastErrorMessage;
        return false;
    }

    qCInfo(lcHikvisionSdk) << "Preview started handle=" << m_previewHandle
                           << "channel=" << channel;
    return true;
}

bool HikvisionSdkService::stopPreview()
{
    if (m_previewHandle < 0) {
        return true;
    }

    if (!NET_DVR_StopRealPlay(m_previewHandle)) {
        setLastError(QStringLiteral("NET_DVR_StopRealPlay"));
        qCWarning(lcHikvisionSdk) << "Stop preview failed:" << m_lastErrorMessage;
        return false;
    }

    m_previewHandle = -1;
    qCInfo(lcHikvisionSdk) << "Preview stopped";
    return true;
}

bool HikvisionSdkService::startPlaybackByTime(int slot, WId winId, int channel,
                                              const QDateTime &start, const QDateTime &end)
{
    if (!ensureInitialized()) {
        return false;
    }
    if (m_userId < 0) {
        setLastError(QStringLiteral("NET_DVR_Login_V30"));
        return false;
    }
    if (!isValidSlot(slot)) {
        setLastError(QStringLiteral("startPlaybackByTime"));
        return false;
    }

    stopPlayback(slot);

    NET_DVR_VOD_PARA vod = {};
    vod.dwSize = sizeof(vod);
    vod.struIDInfo.dwSize = sizeof(vod.struIDInfo);
    vod.struIDInfo.dwChannel = static_cast<DWORD>(channel);
    fillHkTime(start, &vod.struBeginTime);
    fillHkTime(end, &vod.struEndTime);
    vod.hWnd = reinterpret_cast<HWND>(winId);
    vod.byStreamType = 0;

    const long handle = NET_DVR_PlayBackByTime_V40(m_userId, &vod);
    if (handle < 0) {
        setLastError(QStringLiteral("NET_DVR_PlayBackByTime_V40"));
        qCWarning(lcHikvisionSdk) << "Start playback failed:" << m_lastErrorMessage
                                  << "slot=" << slot << "channel=" << channel;
        return false;
    }

    m_playbackHandles[slot] = handle;
    if (!NET_DVR_PlayBackControl_V40(handle, NET_DVR_PLAYSTART, nullptr, 0, nullptr, nullptr)) {
        setLastError(QStringLiteral("NET_DVR_PlayBackControl_V40"));
        qCWarning(lcHikvisionSdk) << "Playback start command failed:" << m_lastErrorMessage
                                  << "slot=" << slot;
        NET_DVR_StopPlayBack(handle);
        m_playbackHandles[slot] = -1;
        return false;
    }

    qCInfo(lcHikvisionSdk) << "Playback started handle=" << handle << "slot=" << slot;
    return true;
}

bool HikvisionSdkService::stopPlayback(int slot)
{
    if (!isValidSlot(slot)) {
        return false;
    }
    if (m_playbackHandles[slot] < 0) {
        return true;
    }

    if (!NET_DVR_StopPlayBack(m_playbackHandles[slot])) {
        setLastError(QStringLiteral("NET_DVR_StopPlayBack"));
        qCWarning(lcHikvisionSdk) << "Stop playback failed:" << m_lastErrorMessage
                                  << "slot=" << slot;
        return false;
    }

    m_playbackHandles[slot] = -1;
    qCInfo(lcHikvisionSdk) << "Playback stopped slot=" << slot;
    return true;
}

bool HikvisionSdkService::stopPlaybackAll()
{
    bool ok = true;
    for (int i = 0; i < 2; ++i) {
        if (!stopPlayback(i)) {
            ok = false;
        }
    }
    return ok;
}

bool HikvisionSdkService::seekPlaybackTime(int slot, const QDateTime &time)
{
    if (!isValidSlot(slot) || m_playbackHandles[slot] < 0) {
        setLastError(QStringLiteral("NET_DVR_PlayBackControl"));
        return false;
    }

    NET_DVR_TIME hkTime = {};
    fillHkTime(time, &hkTime);

    if (!NET_DVR_PlayBackControl_V40(m_playbackHandles[slot], NET_DVR_PLAYSETTIME, &hkTime,
                                     sizeof(NET_DVR_TIME), nullptr, nullptr)) {
        setLastError(QStringLiteral("NET_DVR_PlayBackControl_V40"));
        qCWarning(lcHikvisionSdk) << "Seek playback failed:" << m_lastErrorMessage
                                  << "slot=" << slot;
        return false;
    }

    return true;
}

bool HikvisionSdkService::seekPlaybackTimeAll(const QDateTime &time)
{
    bool ok = true;
    for (int i = 0; i < 2; ++i) {
        if (!seekPlaybackTime(i, time)) {
            ok = false;
        }
    }
    return ok;
}

bool HikvisionSdkService::pausePlayback(int slot, bool pause)
{
    if (!isValidSlot(slot) || m_playbackHandles[slot] < 0) {
        setLastError(QStringLiteral("NET_DVR_PlayBackControl"));
        return false;
    }

    const DWORD cmd = pause ? NET_DVR_PLAYPAUSE : NET_DVR_PLAYSTART;
    if (!NET_DVR_PlayBackControl(m_playbackHandles[slot], cmd, 0, nullptr)) {
        setLastError(QStringLiteral("NET_DVR_PlayBackControl"));
        qCWarning(lcHikvisionSdk) << "Pause/resume failed:" << m_lastErrorMessage
                                  << "slot=" << slot;
        return false;
    }

    return true;
}

bool HikvisionSdkService::pausePlaybackAll(bool pause)
{
    bool ok = true;
    for (int i = 0; i < 2; ++i) {
        if (!pausePlayback(i, pause)) {
            ok = false;
        }
    }
    return ok;
}

bool HikvisionSdkService::setPlaybackSpeed(int slot, double speed)
{
    if (!isValidSlot(slot) || m_playbackHandles[slot] < 0) {
        setLastError(QStringLiteral("NET_DVR_PlayBackControl_V40"));
        return false;
    }

    const auto handle = m_playbackHandles[slot];

    if (!NET_DVR_PlayBackControl_V40(handle, NET_DVR_PLAYNORMAL, nullptr, 0, nullptr, nullptr)) {
        setLastError(QStringLiteral("NET_DVR_PLAYNORMAL"));
        qCWarning(lcHikvisionSdk) << "Set normal speed failed:" << m_lastErrorMessage
                                  << "slot=" << slot;
        return false;
    }

    int steps = 0;
    if (speed > 1.0) {
        if (speed >= 4.0) {
            steps = 2;
        } else {
            steps = 1;
        }
        for (int i = 0; i < steps; ++i) {
            if (!NET_DVR_PlayBackControl_V40(handle, NET_DVR_PLAYFAST, nullptr, 0, nullptr, nullptr)) {
                setLastError(QStringLiteral("NET_DVR_PLAYFAST"));
                qCWarning(lcHikvisionSdk) << "Set fast speed failed:" << m_lastErrorMessage
                                          << "slot=" << slot;
                return false;
            }
        }
        qCInfo(lcHikvisionSdk) << "Speed set to" << speed << "slot=" << slot;
        return true;
    }

    if (speed < 1.0) {
        if (speed <= 0.25) {
            steps = 2;
        } else {
            steps = 1;
        }
        for (int i = 0; i < steps; ++i) {
            if (!NET_DVR_PlayBackControl_V40(handle, NET_DVR_PLAYSLOW, nullptr, 0, nullptr, nullptr)) {
                setLastError(QStringLiteral("NET_DVR_PLAYSLOW"));
                qCWarning(lcHikvisionSdk) << "Set slow speed failed:" << m_lastErrorMessage
                                          << "slot=" << slot;
                return false;
            }
        }
    }

    qCInfo(lcHikvisionSdk) << "Speed set to" << speed << "slot=" << slot;
    return true;
}

bool HikvisionSdkService::setPlaybackSpeedAll(double speed)
{
    bool ok = true;
    for (int i = 0; i < 2; ++i) {
        if (!setPlaybackSpeed(i, speed)) {
            ok = false;
        }
    }
    return ok;
}

bool HikvisionSdkService::hasPlayback(int slot) const
{
    return isValidSlot(slot) && m_playbackHandles[slot] >= 0;
}

bool HikvisionSdkService::hasAnyPlayback() const
{
    return hasPlayback(0) || hasPlayback(1);
}

bool HikvisionSdkService::playbackOsdTime(int slot, QDateTime &time) const
{
    if (!isValidSlot(slot) || m_playbackHandles[slot] < 0) {
        return false;
    }

    NET_DVR_TIME hkTime = {};
    if (!NET_DVR_GetPlayBackOsdTime(m_playbackHandles[slot], &hkTime)) {
        return false;
    }

    time = fromHkTime(&hkTime);
    return time.isValid();
}

QList<HikvisionSdkService::PlaybackFile> HikvisionSdkService::findFilesByChannel(
    int channel, const QDateTime &start, const QDateTime &end, int maxFiles, bool quickSearch)
{
    QList<PlaybackFile> files;
    if (!ensureInitialized()) {
        return files;
    }
    if (m_userId < 0) {
        setLastError(QStringLiteral("NET_DVR_Login_V40"));
        return files;
    }
    if (!start.isValid() || !end.isValid() || start >= end) {
        return files;
    }

    NET_DVR_FILECOND_V40 cond = {};
    cond.lChannel = channel;
    cond.dwFileType = 0xff;
    cond.dwIsLocked = 0xff;
    cond.dwUseCardNo = 0;
    fillHkTime(start, &cond.struStartTime);
    fillHkTime(end, &cond.struStopTime);
    cond.byDrawFrame = 0;
    cond.byFindType = 0;
    cond.byQuickSearch = quickSearch ? 1 : 0;
    cond.byStreamType = 0xff;

    const long handle = NET_DVR_FindFile_V40(m_userId, &cond);
    if (handle < 0) {
        setLastError(QStringLiteral("NET_DVR_FindFile_V40"));
        return files;
    }

    NET_DVR_FINDDATA_V40 data = {};
    while (static_cast<int>(files.size()) < maxFiles) {
        const long status = NET_DVR_FindNextFile_V40(handle, &data);
        if (status == NET_DVR_FILE_SUCCESS) {
            PlaybackFile file;
            file.fileName = QString::fromLocal8Bit(data.sFileName);
            file.startTime = fromHkTime(&data.struStartTime);
            file.endTime = fromHkTime(&data.struStopTime);
            file.fileSize = data.dwFileSize;
            files.push_back(file);
            continue;
        }
        if (status == NET_DVR_FILE_NOFIND) {
            break;
        }
        if (status == NET_DVR_FILE_EXCEPTION) {
            setLastError(QStringLiteral("NET_DVR_FindNextFile_V40"));
            break;
        }
        break;
    }

    NET_DVR_FindClose_V30(handle);
    return files;
}

QList<HikvisionSdkService::PlaybackFile> HikvisionSdkService::findFilesByStreamId(
    const QByteArray &streamId, const QDateTime &start, const QDateTime &end, int maxFiles)
{
    QList<PlaybackFile> files;
    if (!ensureInitialized()) {
        return files;
    }
    if (m_userId < 0) {
        setLastError(QStringLiteral("NET_DVR_Login_V40"));
        return files;
    }
    if (!start.isValid() || !end.isValid() || start >= end) {
        return files;
    }

    NET_DVR_SEARCH_EVENT_PARAM_V40 param = {};
    param.wMajorType = 0xffff;
    param.wMinorType = 0xffff;
    fillHkTime(start, &param.struStartTime);
    fillHkTime(end, &param.struEndTime);
    param.byLockType = 0xff;
    param.byQuickSearch = 1;

    param.uSeniorParam.struStreamIDParam.struIDInfo.dwSize =
        sizeof(param.uSeniorParam.struStreamIDParam.struIDInfo);
    param.uSeniorParam.struStreamIDParam.struIDInfo.dwChannel = 0xffffffff;
    memset(param.uSeniorParam.struStreamIDParam.struIDInfo.byID, 0,
           sizeof(param.uSeniorParam.struStreamIDParam.struIDInfo.byID));
    const int copyLen = qMin<int>(streamId.size(),
                                  sizeof(param.uSeniorParam.struStreamIDParam.struIDInfo.byID));
    if (copyLen > 0) {
        memcpy(param.uSeniorParam.struStreamIDParam.struIDInfo.byID, streamId.constData(),
               copyLen);
    }
    param.uSeniorParam.struStreamIDParam.dwCmdType = 0;
    param.uSeniorParam.struStreamIDParam.byBackupVolumeNum = 0;

    const long handle = NET_DVR_FindFileByEvent_V40(m_userId, &param);
    if (handle < 0) {
        setLastError(QStringLiteral("NET_DVR_FindFileByEvent_V40"));
        return files;
    }

    NET_DVR_SEARCH_EVENT_RET_V40 ret = {};
    while (static_cast<int>(files.size()) < maxFiles) {
        const long status = NET_DVR_FindNextEvent_V40(handle, &ret);
        if (status == NET_DVR_FILE_SUCCESS) {
            PlaybackFile file;
            file.fileName = QString::fromLocal8Bit(
                reinterpret_cast<const char *>(ret.uSeniorRet.struStreamIDRet.byFileNameEx));
            if (file.fileName.isEmpty()) {
                file.fileName = QString::fromLocal8Bit(
                    reinterpret_cast<const char *>(ret.uSeniorRet.struStreamIDRet.byFileName));
            }
            file.startTime = fromHkTime(&ret.struStartTime);
            file.endTime = fromHkTime(&ret.struEndTime);
            file.fileSize = ret.uSeniorRet.struStreamIDRet.dwRecordLength;
            files.push_back(file);
            continue;
        }
        if (status == NET_DVR_FILE_NOFIND) {
            break;
        }
        if (status == NET_DVR_FILE_EXCEPTION) {
            setLastError(QStringLiteral("NET_DVR_FindNextEvent_V40"));
            break;
        }
        break;
    }

    NET_DVR_FindClose_V30(handle);
    return files;
}

int HikvisionSdkService::lastErrorCode() const
{
    return m_lastErrorCode;
}

QString HikvisionSdkService::lastErrorMessage() const
{
    return m_lastErrorMessage;
}

void HikvisionSdkService::setLastError(const QString &action)
{
    m_lastErrorCode = static_cast<int>(NET_DVR_GetLastError());
    m_lastErrorMessage = QStringLiteral("%1 failed, error=%2").arg(action).arg(m_lastErrorCode);
}

bool HikvisionSdkService::ensureInitialized()
{
    if (m_initialized) {
        return true;
    }
    return initialize();
}

void HikvisionSdkService::fillHkTime(const QDateTime &time, void *hkTime)
{
    if (!hkTime) {
        return;
    }

    const QDateTime localTime = time.isValid() ? time.toLocalTime() : QDateTime::currentDateTime();
    const QDate date = localTime.date();
    const QTime tod = localTime.time();

    NET_DVR_TIME *t = static_cast<NET_DVR_TIME *>(hkTime);
    t->dwYear = static_cast<DWORD>(date.year());
    t->dwMonth = static_cast<DWORD>(date.month());
    t->dwDay = static_cast<DWORD>(date.day());
    t->dwHour = static_cast<DWORD>(tod.hour());
    t->dwMinute = static_cast<DWORD>(tod.minute());
    t->dwSecond = static_cast<DWORD>(tod.second());
}

QDateTime HikvisionSdkService::fromHkTime(const void *hkTime)
{
    if (!hkTime) {
        return QDateTime();
    }

    const NET_DVR_TIME *t = static_cast<const NET_DVR_TIME *>(hkTime);
    const QDate date(static_cast<int>(t->dwYear), static_cast<int>(t->dwMonth),
                     static_cast<int>(t->dwDay));
    const QTime tod(static_cast<int>(t->dwHour), static_cast<int>(t->dwMinute),
                    static_cast<int>(t->dwSecond));
    if (!date.isValid() || !tod.isValid()) {
        return QDateTime();
    }

    return QDateTime(date, tod);
}

bool HikvisionSdkService::isValidSlot(int slot)
{
    return slot >= 0 && slot < 2;
}
