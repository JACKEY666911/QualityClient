#pragma once

#include <QString>
#include <QJsonObject>
#include <QObject>


struct LoginRequest
{
    Q_GADGET
public:
    Q_PROPERTY(QString username MEMBER username)
    Q_PROPERTY(QString password MEMBER password)
    Q_PROPERTY(QString deviceCode MEMBER deviceCode)
    Q_PROPERTY(int deviceType MEMBER deviceType)

    QString username;
    QString password;
    QString deviceCode;
    int deviceType = 2000;

    QJsonObject toJson() const;
};

struct ImageDistributeRequest
{
    Q_GADGET
public:
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString deviceCode MEMBER deviceCode)
    Q_PROPERTY(bool firstTime MEMBER firstTime)

    QString id;
    QString deviceCode;
    bool firstTime = false;

    QJsonObject toJson() const;
};

struct UserModel
{
    Q_GADGET
public:
    Q_PROPERTY(QString tokenHead MEMBER tokenHead)
    Q_PROPERTY(QString token MEMBER token)

    QString tokenHead;
    QString token;

    static UserModel fromJson(const QJsonObject &obj);
};
