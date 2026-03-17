#include "XrayImage.h"

XrayImage::XrayImage(QObject *parent)
    : QObject(parent),
      m_qualityResult(-1),
      m_isSelected(false),
      m_viewCount(-1),
      m_checkCountText(QStringLiteral("1次"))
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
    emit dataChanged();
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
    emit dataChanged();
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
    emit dataChanged();
}

int XrayImage::qualityResult() const
{
    return m_qualityResult;
}

void XrayImage::setQualityResult(int value)
{
    if (m_qualityResult == value) {
        return;
    }
    m_qualityResult = value;
    emit dataChanged();
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
    if (m_pbEnhancedType == value) {
        return;
    }
    m_pbEnhancedType = value;
    updateNameFromPbEnhancedType(value);
    emit dataChanged();
}

QList<int> XrayImage::enhancedType() const
{
    return m_enhancedType;
}

void XrayImage::setEnhancedType(const QList<int> &value)
{
    m_enhancedType = value;
    updateNameFromEnhancedType();
    emit dataChanged();
}

bool XrayImage::isSelected() const
{
    return m_isSelected;
}

void XrayImage::setSelected(bool selected)
{
    if (m_isSelected == selected) {
        return;
    }
    m_isSelected = selected;
    emit selectionChanged(m_isSelected);
    emit dataChanged();
}

qint64 XrayImage::viewCount() const
{
    return m_viewCount;
}

void XrayImage::setViewCount(qint64 count)
{
    if (m_viewCount == count) {
        return;
    }
    m_viewCount = count;
    updateNameWithViewCount(m_name);
    emit dataChanged();
}

QString XrayImage::checkCountText() const
{
    return m_checkCountText;
}

void XrayImage::setCheckCountText(const QString &text)
{
    if (m_checkCountText == text) {
        return;
    }
    m_checkCountText = text;
    emit dataChanged();
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
            emit nameChanged(m_name);
            return;
        }
        updateNameWithViewCount(QStringLiteral("1次"));
    }
}

void XrayImage::updateNameWithViewCount(const QString &fallback)
{
    if (m_viewCount > 0) {
        m_name = QStringLiteral("%1次").arg(m_viewCount);
    } else {
        m_name = fallback;
    }
    emit nameChanged(m_name);
}
