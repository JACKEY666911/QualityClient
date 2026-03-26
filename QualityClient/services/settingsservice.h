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

#pragma once

#include <QHash>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QStandardPaths>
#include <QDir>

#define SETTINGS_INI_PATH \
    QCoreApplication::applicationDirPath() + "/settings.ini"

class SettingsService : public QObject {
    Q_OBJECT

   public:
    static SettingsService &instance();

    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    void setValue(const QString &key, const QVariant &value);
    void remove(const QString &key);
    bool contains(const QString &key) const;
    void sync();
    QStringList allKeys() const;
    void clear();

    void beginGroup(const QString &prefix);
    void endGroup();
    QString group() const;
    QString fileName() const;
    void beginWriteArray(const QString &prefix, int size);
    void setArrayIndex(int i);
    void endArray();
    int beginReadArray(const QString &prefix);

    // Typed settings accessors
    void loadSetting(const QString &settingsPath = QString());
    void ensureShortcutDefaults();
    QString loadShortcut(const QString &key, const QString &fallback = QString()) const;

    template <typename T>
    T loadSetting(const QString &key, const T &defaultValue) const
    {
        return value(key, QVariant::fromValue(defaultValue)).template value<T>();
    }

    int maxBindPersons() const;
    QString baseUrl() const;
    int qcCountdownSec() const;
    QString mqHostName() const;
    QString mqUserName() const;
    QString mqPassword() const;
    int mqPort() const;
    int historyQueryCount() const;
    QString aiContrabandsColor() const;
    int enhancedImageTimeOut() const;
    int inspectionLowThreshold() const;
    int inspectionHighThreshold() const;
    int commonTimeOut() const;
    int taskFetchTime() const;
    int videoBackTimeSpan() const;
    QString deviceCode() const;
    QString airportCode() const;
    QString areaCode() const;

    // Delete copy constructor and assignment operator
    SettingsService(const SettingsService &) = delete;
    SettingsService &operator=(const SettingsService &) = delete;

    explicit SettingsService(QObject *parent = nullptr);
    ~SettingsService() override = default;

   private:
    QSettings m_settings;
    QString m_group;
    int m_arrayIndex;
    QStringList m_arrayStack;

    static QHash<QString, QVariant> *cache();
    static const QHash<QString, QString> &shortcutDefaults();
    QString getFullKey(const QString &key) const;
};
