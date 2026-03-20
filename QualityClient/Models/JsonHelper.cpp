#include "JsonHelper.h"

#include <QJsonArray>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QVariant>
#include <QDateTime>

QJsonObject JsonHelper::toJsonObject(const QObject *object)
{
    if (!object) {
        return {};
    }
    const QMetaObject *meta = object->metaObject();
    const QHash<QString, QString> fieldMap = buildFieldMap(meta);
    QJsonObject json;

    for (int i = 0; i < meta->propertyCount(); ++i) {
        const QMetaProperty prop = meta->property(i);
        if (!prop.isReadable()) {
            continue;
        }
        const char *propName = prop.name();
        const QString jsonKey = fieldMap.value(QString::fromLatin1(propName), QString::fromLatin1(propName));
        const QVariant value = prop.read(object);
        json.insert(jsonKey, variantToJson(value, prop.userType()));
    }
    return json;
}

QJsonObject JsonHelper::toJsonObject(const void *gadget, const QMetaObject *meta)
{
    if (!gadget || !meta) {
        return {};
    }

    const QHash<QString, QString> fieldMap = buildFieldMap(meta);
    QJsonObject json;

    for (int i = 0; i < meta->propertyCount(); ++i) {
        const QMetaProperty prop = meta->property(i);
        if (!prop.isReadable()) {
            continue;
        }
        const char *propName = prop.name();
        const QString jsonKey = fieldMap.value(QString::fromLatin1(propName), QString::fromLatin1(propName));
        QVariant value;
        if (prop.isReadable()) {
            value = prop.readOnGadget(gadget);
        }
        json.insert(jsonKey, variantToJson(value, prop.userType()));
    }
    return json;
}

bool JsonHelper::fromJsonObject(QObject *object, const QJsonObject &json)
{
    if (!object) {
        return false;
    }
    const QMetaObject *meta = object->metaObject();
    const QHash<QString, QString> fieldMap = buildFieldMap(meta);
    bool anySet = false;

    for (int i = 0; i < meta->propertyCount(); ++i) {
        const QMetaProperty prop = meta->property(i);
        if (!prop.isWritable()) {
            continue;
        }
        const QString propName = QString::fromLatin1(prop.name());
        const QString jsonKey = fieldMap.value(propName, propName);
        if (!json.contains(jsonKey)) {
            continue;
        }
        const QJsonValue jsonVal = json.value(jsonKey);
        const QVariant value = jsonToVariant(jsonVal, prop.userType());
        if (value.isValid()) {
            prop.write(object, value);
            anySet = true;
        }
    }
    return anySet;
}

bool JsonHelper::fromJsonObject(void *gadget, const QMetaObject *meta, const QJsonObject &json)
{
    if (!gadget || !meta) {
        return false;
    }

    const QHash<QString, QString> fieldMap = buildFieldMap(meta);
    bool anySet = false;

    for (int i = 0; i < meta->propertyCount(); ++i) {
        const QMetaProperty prop = meta->property(i);
        if (!prop.isWritable()) {
            continue;
        }
        const QString propName = QString::fromLatin1(prop.name());
        const QString jsonKey = fieldMap.value(propName, propName);
        if (!json.contains(jsonKey)) {
            continue;
        }
        const QJsonValue jsonVal = json.value(jsonKey);
        const QVariant value = jsonToVariant(jsonVal, prop.userType());
        if (value.isValid()) {
            prop.writeOnGadget(gadget, value);
            anySet = true;
        }
    }
    return anySet;
}

QHash<QString, QString> JsonHelper::buildFieldMap(const QMetaObject *meta)
{
    QHash<QString, QString> map;
    if (!meta) {
        return map;
    }
    for (int i = 0; i < meta->classInfoCount(); ++i) {
        const QMetaClassInfo info = meta->classInfo(i);
        if (QString::fromLatin1(info.name()) == QStringLiteral("json")) {
            const QString text = QString::fromLatin1(info.value());
            const QStringList pairs = text.split(QLatin1Char(';'), Qt::SkipEmptyParts);
            for (const QString &pair : pairs) {
                const int eq = pair.indexOf(QLatin1Char('='));
                if (eq > 0) {
                    const QString key = pair.left(eq).trimmed();
                    const QString val = pair.mid(eq + 1).trimmed();
                    if (!key.isEmpty() && !val.isEmpty()) {
                        map.insert(key, val);
                    }
                }
            }
        }
    }
    return map;
}

QJsonValue JsonHelper::variantToJson(const QVariant &value, int typeId)
{
    if (!value.isValid()) {
        return QJsonValue();
    }

    if (typeId == QMetaType::Bool) {
        return QJsonValue(value.toBool());
    }
    if (typeId == QMetaType::Int || typeId == QMetaType::LongLong || typeId == QMetaType::UInt || typeId == QMetaType::ULongLong) {
        return QJsonValue(value.toLongLong());
    }
    if (typeId == QMetaType::Double || typeId == QMetaType::Float) {
        return QJsonValue(value.toDouble());
    }
    if (typeId == QMetaType::QString) {
        return QJsonValue(value.toString());
    }
    if (typeId == QMetaType::QDateTime) {
        return QJsonValue(value.toDateTime().toString(Qt::ISODateWithMs));
    }
    if (typeId == QMetaType::QByteArray) {
        return QJsonValue(QString::fromUtf8(value.toByteArray()));
    }
    if (typeId == QMetaType::QJsonObject) {
        return QJsonValue(value.toJsonObject());
    }
    if (typeId == QMetaType::QJsonArray) {
        return QJsonValue(value.toJsonArray());
    }
    if (typeId == QMetaType::QStringList) {
        QJsonArray arr;
        const QStringList list = value.toStringList();
        for (const QString &item : list) {
            arr.append(item);
        }
        return arr;
    }
    if (typeId == QMetaType::QVariantList) {
        QJsonArray arr;
        const QVariantList list = value.toList();
        for (const QVariant &item : list) {
            arr.append(variantToJson(item, item.userType()));
        }
        return arr;
    }
    if (typeId == qMetaTypeId<QList<int>>()) {
        QJsonArray arr;
        const QList<int> list = value.value<QList<int>>();
        for (int v : list) {
            arr.append(v);
        }
        return arr;
    }
    if (typeId == qMetaTypeId<QList<QString>>()) {
        QJsonArray arr;
        const QList<QString> list = value.value<QList<QString>>();
        for (const QString &v : list) {
            arr.append(v);
        }
        return arr;
    }

    const QMetaType metaType(typeId);
    const QMetaObject *meta = metaType.metaObject();
    if (meta && metaType.flags().testFlag(QMetaType::IsGadget)) {
        return QJsonValue(toJsonObject(value.constData(), meta));
    }

    return QJsonValue::fromVariant(value);
}

QVariant JsonHelper::jsonToVariant(const QJsonValue &value, int typeId)
{
    if (value.isUndefined() || value.isNull()) {
        return {};
    }

    if (typeId == QMetaType::Bool) {
        return QVariant(value.toBool());
    }
    if (typeId == QMetaType::Int) {
        return QVariant(value.toInt());
    }
    if (typeId == QMetaType::LongLong || typeId == QMetaType::ULongLong) {
        return QVariant::fromValue(value.toVariant().toLongLong());
    }
    if (typeId == QMetaType::Double || typeId == QMetaType::Float) {
        return QVariant(value.toDouble());
    }
    if (typeId == QMetaType::QString) {
        return QVariant(value.toString());
    }
    if (typeId == QMetaType::QDateTime) {
        // return QVariant(QDateTime::fromString(value.toString(), Qt::ISODateWithMs));
        QString str = value.toString();
        QDateTime dt = QDateTime::fromString(str, Qt::ISODateWithMs);
        if (!dt.isValid()) {
            dt = QDateTime::fromString(str, Qt::ISODate); // 尝试不带毫秒的格式
        }
        return QVariant(dt);
    }
    if (typeId == QMetaType::QByteArray) {
        return QVariant(value.toString().toUtf8());
    }
    if (typeId == QMetaType::QJsonObject) {
        return QVariant(value.toObject());
    }
    if (typeId == QMetaType::QJsonArray) {
        return QVariant(value.toArray());
    }
    if (typeId == QMetaType::QStringList) {
        QStringList list;
        const QJsonArray arr = value.toArray();
        for (const QJsonValue &v : arr) {
            list.append(v.toString());
        }
        return QVariant(list);
    }
    if (typeId == QMetaType::QVariantList) {
        QVariantList list;
        const QJsonArray arr = value.toArray();
        for (const QJsonValue &v : arr) {
            list.append(v.toVariant());
        }
        return QVariant(list);
    }
    if (typeId == qMetaTypeId<QList<int>>()) {
        QList<int> list;
        const QJsonArray arr = value.toArray();
        for (const QJsonValue &v : arr) {
            list.append(v.toInt());
        }
        return QVariant::fromValue(list);
    }
    if (typeId == qMetaTypeId<QList<QString>>()) {
        QList<QString> list;
        const QJsonArray arr = value.toArray();
        for (const QJsonValue &v : arr) {
            list.append(v.toString());
        }
        return QVariant::fromValue(list);
    }

    const QMetaType metaType(typeId);
    const QMetaObject *meta = metaType.metaObject();
    if (meta && metaType.flags().testFlag(QMetaType::IsGadget)) {
        QVariant out(typeId);
        void *ptr = out.data();
        if (ptr && fromJsonObject(ptr, meta, value.toObject())) {
            return out;
        }
    }

    return value.toVariant();
}
