#pragma once

#include <QJsonObject>
#include <QMetaObject>

class QObject;

class JsonHelper
{
public:
    static QJsonObject toJsonObject(const QObject *object);
    static QJsonObject toJsonObject(const void *gadget, const QMetaObject *meta);

    static bool fromJsonObject(QObject *object, const QJsonObject &json);
    static bool fromJsonObject(void *gadget, const QMetaObject *meta, const QJsonObject &json);

    template<typename T>
    static QJsonObject toJsonObject(const T &gadget)
    {
        return toJsonObject(&gadget, &T::staticMetaObject);
    }

    template<typename T>
    static bool fromJsonObject(T &gadget, const QJsonObject &json)
    {
        return fromJsonObject(&gadget, &T::staticMetaObject, json);
    }

private:
    static QHash<QString, QString> buildFieldMap(const QMetaObject *meta);
    static QJsonValue variantToJson(const QVariant &value, int typeId);
    static QVariant jsonToVariant(const QJsonValue &value, int typeId);
};
