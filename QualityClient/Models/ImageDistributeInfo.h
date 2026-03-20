#pragma once

#include <QString>
#include <QList>
#include <QtGlobal>
#include <QJsonObject>
#include <QJsonArray>
#include <QObject>

#include "Models/XrayImage.h"

struct ImageDistributeInfo
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(int type MEMBER type)
    Q_PROPERTY(QString channelNo MEMBER channelNo)
    Q_PROPERTY(QString channelNoValue MEMBER channelNoValue)
    Q_PROPERTY(QString passengerName MEMBER passengerName)
    Q_PROPERTY(QString flightNo MEMBER flightNo)
    Q_PROPERTY(QString flightDate MEMBER flightDate)
    Q_PROPERTY(QString seat MEMBER seat)
    Q_PROPERTY(QString destination MEMBER destination)
    Q_PROPERTY(QString passageway MEMBER passageway)
    Q_PROPERTY(QString baggageNo MEMBER baggageNo)
    Q_PROPERTY(QString checkTime MEMBER checkTime)
    Q_PROPERTY(int imageResult MEMBER imageResult)
    Q_PROPERTY(QVariant recheckResult MEMBER recheckResult)
    Q_PROPERTY(QVariant qualityResult MEMBER qualityResult)
    Q_PROPERTY(QString qualityControlTime MEMBER qualityControlTime)
    Q_PROPERTY(QVariant aiQcResult MEMBER aiQcResult)
    Q_PROPERTY(QVariant enhancedTypeCount MEMBER enhancedTypeCount)
    Q_PROPERTY(QVariant taskCount MEMBER taskCount)
    Q_PROPERTY(QString timeInterval MEMBER timeInterval)
    Q_PROPERTY(QVariant tagResult MEMBER tagResult)
    Q_PROPERTY(QString freshness MEMBER freshness)
    Q_PROPERTY(QVariant viewDuration MEMBER viewDuration)
    Q_PROPERTY(QVariant reExtractStatus MEMBER reExtractStatus)

    QString id;
    int type = -1;
    QString channelNo;
    QString channelNoValue;
    QString passengerName;
    QString flightNo;
    QString flightDate;
    QString seat;
    QString destination;
    QString passageway;
    QString baggageNo;
    QString checkTime;
    int imageResult = -1;
    QVariant recheckResult;
    QVariant qualityResult;
    QString qualityControlTime;
    QVariant aiQcResult;
    QVariant enhancedTypeCount;
    QVariant taskCount;
    QString timeInterval;
    QList<XrayImage> xrayImages;
    QList<XrayImage> pbXrayImages;
    QVariant tagResult;
    QString freshness;
    QVariant viewDuration;
    QVariant reExtractStatus;

    static ImageDistributeInfo fromJson(const QJsonObject &obj);
};
