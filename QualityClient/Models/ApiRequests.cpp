#include "ApiRequests.h"

#include "Models/JsonHelper.h"

QJsonObject LoginRequest::toJson() const
{
    return JsonHelper::toJsonObject(*this);
}

QJsonObject ImageDistributeRequest::toJson() const
{
    return JsonHelper::toJsonObject(*this);
}

UserModel UserModel::fromJson(const QJsonObject &obj)
{
    UserModel model;
    JsonHelper::fromJsonObject(model, obj);
    return model;
}
