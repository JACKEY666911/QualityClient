#include "XrayImage.h"

#include <QJsonArray>

XrayImage::XrayImage()
    : m_qualityResult(QVariant())
    , m_isSelected(false)
    , m_viewCount(QVariant())
    , m_checkCountText(QStringLiteral("1次"))
{
}

QString XrayImage::fullXrayImageUrl() const
{
    return m_fullXrayImageUrl;
}

void XrayImage::setFullXrayImageUrl(const QString &url)
{
    if (m_fullXrayImageUrl == url) {
        return;
    }
    m_fullXrayImageUrl = url;
}

QString XrayImage::mainXrayImageUrl() const
{
    return m_mainXrayImageUrl;
}

void XrayImage::setMainXrayImageUrl(const QString &url)
{
    if (m_mainXrayImageUrl == url) {
        return;
    }
    m_mainXrayImageUrl = url;
}

QString XrayImage::assistXrayImageUrl() const
{
    return m_assistXrayImageUrl;
}

void XrayImage::setAssistXrayImageUrl(const QString &url)
{
    if (m_assistXrayImageUrl == url) {
        return;
    }
    m_assistXrayImageUrl = url;
}

int XrayImage::qualityResult() const
{
    if (!m_qualityResult.isValid()) {
        return -1;
    }
    return m_qualityResult.toInt();
}

void XrayImage::setQualityResult(int value)
{
    m_qualityResult = value;
}

QString XrayImage::name() const
{
    return m_name;
}

QString XrayImage::pbEnhancedType() const
{
    return m_pbEnhancedType;
}

void XrayImage::setPbEnhancedType(const QString &value)
{
    m_pbEnhancedType = value;
    updateNameFromPbEnhancedType(value);
}

QList<int> XrayImage::enhancedType() const
{
    return m_enhancedType;
}

void XrayImage::setEnhancedType(const QList<int> &value)
{
    m_enhancedType = value;
    updateNameFromEnhancedType();
}

bool XrayImage::isSelected() const
{
    return m_isSelected;
}

void XrayImage::setSelected(bool selected)
{
    m_isSelected = selected;
}

qint64 XrayImage::viewCount() const
{
    if (!m_viewCount.isValid()) {
        return -1;
    }
    return m_viewCount.toLongLong();
}

void XrayImage::setViewCount(qint64 count)
{
    m_viewCount = count;
    updateNameWithViewCount(m_name);
}

QString XrayImage::checkCountText() const
{
    return m_checkCountText;
}

void XrayImage::setCheckCountText(const QString &text)
{
    m_checkCountText = text;
}

void XrayImage::updateNameFromEnhancedType()
{
    if (m_enhancedType.size() == 1) {
        updateNameWithViewCount(QStringLiteral("1次"));
        return;
    }
}

void XrayImage::updateNameFromPbEnhancedType(const QString &value)
{
    if (!value.isEmpty()) {
        bool ok = false;
        int intValue = value.toInt(&ok);
        if (ok && intValue >= 800 && intValue < 900) {
            m_name = QStringLiteral("放大");
            return;
        }
        updateNameWithViewCount(QStringLiteral("1次"));
    }
}

void XrayImage::updateNameWithViewCount(const QString &fallback)
{
    const qint64 count = viewCount();
    if (count > 0) {
        m_name = QStringLiteral("%1次").arg(count);
    } else {
        m_name = fallback;
    }
}

XrayImage XrayImage::fromJson(const QJsonObject &obj)
{
    XrayImage img;
    img.setFullXrayImageUrl(obj.value(QStringLiteral("fullXrayImageUrl")).toString());
    img.setMainXrayImageUrl(obj.value(QStringLiteral("mainXrayImageUrl")).toString());
    img.setAssistXrayImageUrl(obj.value(QStringLiteral("assistXrayImageUrl")).toString());
    if (obj.contains(QStringLiteral("qualityResult"))) {
        img.setQualityResult(obj.value(QStringLiteral("qualityResult")).toInt(-1));
    }
    img.setPbEnhancedType(obj.value(QStringLiteral("pbEnhancedType")).toString());

    QList<int> enhanced;
    const QJsonArray enhancedArr = obj.value(QStringLiteral("enhancedType")).toArray();
    for (const QJsonValue &ev : enhancedArr) {
        enhanced.append(ev.toInt());
    }
    if (!enhanced.isEmpty()) {
        img.setEnhancedType(enhanced);
    }
    if (obj.contains(QStringLiteral("viewCount"))) {
        img.setViewCount(static_cast<qint64>(obj.value(QStringLiteral("viewCount")).toDouble(-1)));
    }
    return img;
}
