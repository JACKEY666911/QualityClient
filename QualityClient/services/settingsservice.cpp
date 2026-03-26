/*
 * Copyright (c) 2014-2025 Patrizio Bekerle -- <patrizio@bekerle.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 */

#include "settingsservice.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QtGlobal>

namespace {
QString normalizeIniCode(QString value)
{
    return value.trimmed();
}

QString stripInlineComment(QString value)
{
    const int idx = value.indexOf(QStringLiteral("//"));
    if (idx >= 0) {
        value.truncate(idx);
    }
    return value.trimmed();
}

int readIntWithComment(const SettingsService &settings, const QString &key, int defaultValue)
{
    const QVariant raw = settings.value(key, defaultValue);
    bool ok = false;
    const int directValue = raw.toInt(&ok);
    if (ok) {
        return directValue;
    }

    const QString text = stripInlineComment(normalizeIniCode(raw.toString()));
    const int parsedValue = text.toInt(&ok);
    return ok ? parsedValue : defaultValue;
}

QString readStringWithComment(const SettingsService &settings, const QString &key, const QString &defaultValue)
{
    const QString raw = settings.value(key, defaultValue).toString();
    return stripInlineComment(normalizeIniCode(raw));
}
}

SettingsService::SettingsService(QObject *parent)
    : QObject(parent)
    , m_settings(SETTINGS_INI_PATH, QSettings::IniFormat)
    , m_arrayIndex(-1)
{
    QDir configDir(QFileInfo(SETTINGS_INI_PATH).absolutePath());
    if (!configDir.exists()) {
        configDir.mkpath(".");
    }
    ensureShortcutDefaults();
    m_settings.sync();

    loadSetting(m_settings.fileName());
}

SettingsService &SettingsService::instance() {
    static SettingsService instance;
    return instance;
}

QHash<QString, QVariant> *SettingsService::cache() {
    static QHash<QString, QVariant> cache;
    return &cache;
}

QVariant SettingsService::value(const QString &key, const QVariant &defaultValue) const {
    const QString fullKey = getFullKey(key);

    if (!cache()->contains(fullKey)) {
        cache()->insert(fullKey, m_settings.value(key, defaultValue));
    }

    return cache()->value(fullKey);
}

void SettingsService::setValue(const QString &key, const QVariant &value) {
    const QString fullKey = getFullKey(key);
    cache()->insert(fullKey, value);
    m_settings.setValue(key, value);
}

void SettingsService::remove(const QString &key) {
    const QString fullKey = getFullKey(key);

    if (key.isEmpty() && !m_group.isEmpty()) {
        for (auto it = cache()->begin(); it != cache()->end();) {
            if (it.key().startsWith(fullKey)) {
                it = cache()->erase(it);
            } else {
                ++it;
            }
        }

        m_settings.remove(QLatin1String(""));
    } else {
        cache()->remove(fullKey);
        m_settings.remove(key);
    }
}

bool SettingsService::contains(const QString &key) const {
    const QString fullKey = getFullKey(key);
    return cache()->contains(fullKey) || m_settings.contains(key);
}

QString SettingsService::getFullKey(const QString &key) const {
    QString fullKey = m_group.isEmpty() ? key : m_group + '/' + key;

    if (!m_arrayStack.isEmpty()) {
        fullKey = m_arrayStack.last() + '/' + QString::number(m_arrayIndex) + '/' + fullKey;
    }

    return fullKey;
}

void SettingsService::sync() { m_settings.sync(); }

QStringList SettingsService::allKeys() const { return m_settings.allKeys(); }

void SettingsService::clear() {
    cache()->clear();
    m_settings.clear();
}

void SettingsService::beginGroup(const QString &prefix) {
    m_group = m_group.isEmpty() ? prefix : m_group + '/' + prefix;
    m_settings.beginGroup(prefix);
}

void SettingsService::endGroup() {
    int lastSeparator = m_group.lastIndexOf('/');
    m_group = lastSeparator != -1 ? m_group.left(lastSeparator) : QString();
    m_settings.endGroup();
}

QString SettingsService::group() const { return m_group; }

QString SettingsService::fileName() const { return m_settings.fileName(); }

void SettingsService::beginWriteArray(const QString &prefix, int size) {
    m_arrayStack.append(prefix);
    m_arrayIndex = 0;
    m_settings.beginWriteArray(prefix, size);
}

void SettingsService::setArrayIndex(int i) {
    m_arrayIndex = i;
    m_settings.setArrayIndex(i);
}

void SettingsService::endArray() {
    m_arrayStack.removeLast();
    m_arrayIndex = -1;
    m_settings.endArray();
}

int SettingsService::beginReadArray(const QString &prefix) {
    m_arrayStack.append(prefix);
    m_arrayIndex = 0;
    return m_settings.beginReadArray(prefix);
}

void SettingsService::loadSetting(const QString &settingsPath)
{
    if (!settingsPath.isEmpty() && QFileInfo(settingsPath).absoluteFilePath() != QFileInfo(m_settings.fileName()).absoluteFilePath()) {
        qWarning() << "SettingsService::loadSetting path mismatch, using existing file:"
                   << m_settings.fileName() << "requested:" << settingsPath;
    }

    m_settings.sync();
    cache()->clear();
    const QStringList keys = m_settings.allKeys();
    for (const QString &key : keys) {
        cache()->insert(key, m_settings.value(key));
    }
}

void SettingsService::ensureShortcutDefaults()
{
    const auto &defaults = shortcutDefaults();
    for (auto it = defaults.cbegin(); it != defaults.cend(); ++it) {
        if (!m_settings.contains(it.key())) {
            m_settings.setValue(it.key(), it.value());
        }
    }
}

QString SettingsService::loadShortcut(const QString &key, const QString &fallback) const
{
    const QString defaultValue = fallback.isEmpty()
        ? shortcutDefaults().value(key, QString())
        : fallback;
    return loadSetting<QString>(key, defaultValue);
}

int SettingsService::maxBindPersons() const
{
    return loadSetting<int>(QStringLiteral("maxBindPersons"), 10);
}

QString SettingsService::baseUrl() const
{
    return loadSetting<QString>(QStringLiteral("baseUrl"),
                                QStringLiteral("http://192.168.1.65:17301/")).trimmed();
}

int SettingsService::qcCountdownSec() const
{
    const int fallback = readIntWithComment(*this, QStringLiteral("qcCountdownSec"), 600);
    const int val = readIntWithComment(*this, QStringLiteral("CoreConfig/QcCountdownSec"), fallback);
    return qMax(0, val);
}

QString SettingsService::mqHostName() const
{
    return normalizeIniCode(loadSetting<QString>(QStringLiteral("MQConfig/HostName"), QString()));
}

QString SettingsService::mqUserName() const
{
    return normalizeIniCode(loadSetting<QString>(QStringLiteral("MQConfig/UserName"), QString()));
}

QString SettingsService::mqPassword() const
{
    return normalizeIniCode(loadSetting<QString>(QStringLiteral("MQConfig/Password"), QString()));
}

int SettingsService::mqPort() const
{
    return qMax(0, readIntWithComment(*this, QStringLiteral("MQConfig/Port"), 5672));
}

int SettingsService::historyQueryCount() const
{
    return qMax(0, readIntWithComment(*this, QStringLiteral("CoreConfig/HistoryQueryCount"), 3));
}

QString SettingsService::aiContrabandsColor() const
{
    return readStringWithComment(*this, QStringLiteral("CoreConfig/AiContrabandsColor"),
                                 QStringLiteral("#6F66FF"));
}

int SettingsService::enhancedImageTimeOut() const
{
    return qMax(0, readIntWithComment(*this, QStringLiteral("CoreConfig/EnhancedImageTimeOut"), 30));
}

int SettingsService::inspectionLowThreshold() const
{
    return qMax(0, readIntWithComment(*this, QStringLiteral("CoreConfig/InspectionLowThreshold"), 10));
}

int SettingsService::inspectionHighThreshold() const
{
    return qMax(0, readIntWithComment(*this, QStringLiteral("CoreConfig/InspectionHighThreshold"), 1515));
}

int SettingsService::commonTimeOut() const
{
    return qMax(0, readIntWithComment(*this, QStringLiteral("CoreConfig/CommonTimeOut"), 30));
}

int SettingsService::taskFetchTime() const
{
    return qMax(0, readIntWithComment(*this, QStringLiteral("CoreConfig/TaskFetchTime"), 2));
}

int SettingsService::videoBackTimeSpan() const
{
    return qMax(0, readIntWithComment(*this, QStringLiteral("CoreConfig/VideoBackTimeSpan"), 5));
}

QString SettingsService::deviceCode() const
{
    const QString raw = loadSetting<QString>(QStringLiteral("Device/DeviceCode"),
                                             QStringLiteral("PC-CLIENT-001"));
    return normalizeIniCode(raw);
}

QString SettingsService::airportCode() const
{
    const QString raw = loadSetting<QString>(QStringLiteral("Device/AirportCode"),
                                             QString());
    return normalizeIniCode(raw);
}

QString SettingsService::areaCode() const
{
    const QString raw = loadSetting<QString>(QStringLiteral("Device/AreaCode"),
                                             QString());
    return normalizeIniCode(raw);
}

const QHash<QString, QString> &SettingsService::shortcutDefaults()
{
    static const QHash<QString, QString> defaults = {
        {QStringLiteral("Shortcuts/TF_ED"), QStringLiteral("W")},
        {QStringLiteral("Shortcuts/TF_GEN"), QStringLiteral("Q")},
        {QStringLiteral("Shortcuts/TF_HI"), QStringLiteral("E")},
        {QStringLiteral("Shortcuts/TF_LOW"), QStringLiteral("R")},
        {QStringLiteral("Shortcuts/TF_OS"), QStringLiteral("3")},
        {QStringLiteral("Shortcuts/TF_MS"), QStringLiteral("4")},
        {QStringLiteral("Shortcuts/YS_E0"), QStringLiteral("1")},
        {QStringLiteral("Shortcuts/YS_E1"), QStringLiteral("2")},
        {QStringLiteral("Shortcuts/YS_E2"), QStringLiteral("3")},
        {QStringLiteral("Shortcuts/YS_OS"), QStringLiteral("Q")},
        {QStringLiteral("Shortcuts/YS_HD"), QStringLiteral("W")},
        {QStringLiteral("Shortcuts/YS_SC"), QStringLiteral("E")},
        {QStringLiteral("Shortcuts/ESC"), QStringLiteral("Esc")},
        {QStringLiteral("Shortcuts/StartCheck"), QStringLiteral("F")},
        {QStringLiteral("Shortcuts/Pass"), QStringLiteral("Space")},
        {QStringLiteral("Shortcuts/PersonDetail"), QStringLiteral("`")}
    };
    return defaults;
}
