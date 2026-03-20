#include "ImageDistributeInfo.h"

#include "Models/XrayImage.h"
#include "Models/JsonHelper.h"
ImageDistributeInfo ImageDistributeInfo::fromJson(const QJsonObject &obj)
{
    ImageDistributeInfo info;
    JsonHelper::fromJsonObject(info, obj);

    const QJsonArray xrayArray = obj.value(QStringLiteral("xrayImages")).toArray();
    for (const QJsonValue &v : xrayArray) {
        info.xrayImages.append(XrayImage::fromJson(v.toObject()));
    }

    const QJsonArray pbArray = obj.value(QStringLiteral("pbXrayImages")).toArray();
    for (const QJsonValue &v : pbArray) {
        info.pbXrayImages.append(XrayImage::fromJson(v.toObject()));
    }

    return info;
}
