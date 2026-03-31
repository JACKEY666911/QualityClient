#ifndef QUERYINFO_H
#define QUERYINFO_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include "JsonHelper.h"
#include "XrayImage.h"

struct QueryInfo
{
    Q_GADGET

    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(int type MEMBER type)
    Q_PROPERTY(QString channelNo MEMBER channelNo)
    Q_PROPERTY(QString channelNoValue MEMBER channelNoValue)
    Q_PROPERTY(QString checkTime MEMBER checkTime)
    Q_PROPERTY(int imageResult MEMBER imageResult)
    Q_PROPERTY(int qualityResult MEMBER qualityResult)
    Q_PROPERTY(XrayImage xrayImage MEMBER xrayImage)
    Q_PROPERTY(QString qualityTime MEMBER qualityTime)
    Q_PROPERTY(bool isSelected MEMBER isSelected)
    Q_PROPERTY(int index MEMBER index)
    Q_PROPERTY(bool recheckResult MEMBER recheckResult)

    Q_PROPERTY(QString recheckResultText READ getRecheckResultText)

public:
    QString id;
    int type = -1;
    QString channelNo;
    QString channelNoValue;
    QString checkTime;
    int imageResult = 0;
    int qualityResult = 0;
    XrayImage xrayImage;
    QString qualityTime;
    bool isSelected = false;
    int index = -1;

    int checkUserId = 0;
    QString checkNickName;
    int qcUserId = 0;
    QString qcNickName;
    int qualityOption = 0;
    QDateTime queryStartTime;
    QDateTime queryEndTime;
    int pageNum = 0;
    int pageSize = 0;
    bool recheckResult = false;
    QString getRecheckResultText() const {
        return recheckResult ? "复检" : "首检";
    }

    static QueryInfo fromJson(const QJsonObject &obj)
    {
        QueryInfo info;
        JsonHelper::fromJsonObject(info, obj);
        return info;
    }
};

Q_DECLARE_METATYPE(QueryInfo*)

#endif
