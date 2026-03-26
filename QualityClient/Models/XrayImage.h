#ifndef XRAYIMAGE_H
#define XRAYIMAGE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QMetaType>

class XrayImage
{
    Q_GADGET
    Q_PROPERTY(QString fullXrayImageUrl MEMBER m_fullXrayImageUrl)
    Q_PROPERTY(QString mainXrayImageUrl MEMBER m_mainXrayImageUrl)
    Q_PROPERTY(QString assistXrayImageUrl MEMBER m_assistXrayImageUrl)
    Q_PROPERTY(QVariant qualityResult MEMBER m_qualityResult)
    Q_PROPERTY(QString pbEnhancedType MEMBER m_pbEnhancedType)
    Q_PROPERTY(QList<int> enhancedType MEMBER m_enhancedType)
    Q_PROPERTY(QVariant viewCount MEMBER m_viewCount)
public:
    XrayImage();

    QString fullXrayImageUrl() const;
    void setFullXrayImageUrl(const QString &url);

    QString mainXrayImageUrl() const;
    void setMainXrayImageUrl(const QString &url);

    QString assistXrayImageUrl() const;
    void setAssistXrayImageUrl(const QString &url);

    int qualityResult() const;
    void setQualityResult(int value);

    QString name() const;

    QString pbEnhancedType() const;
    void setPbEnhancedType(const QString &value);

    QList<int> enhancedType() const;
    void setEnhancedType(const QList<int> &value);

    bool isSelected() const;
    void setSelected(bool selected);

    qint64 viewCount() const;
    void setViewCount(qint64 count);

    QString checkCountText() const;
    void setCheckCountText(const QString &text);

    static XrayImage fromJson(const QJsonObject &obj);

    bool operator==(const XrayImage &other) const
    {
        return m_mainXrayImageUrl == other.m_mainXrayImageUrl
            && m_assistXrayImageUrl == other.m_assistXrayImageUrl;
    }

    bool operator!=(const XrayImage &other) const
    {
        return !(*this == other);
    }

private:
    void updateNameFromEnhancedType();
    void updateNameFromPbEnhancedType(const QString &value);
    void updateNameWithViewCount(const QString &fallback);

    QString m_fullXrayImageUrl;
    QString m_mainXrayImageUrl;
    QString m_assistXrayImageUrl;
    QVariant m_qualityResult;
    QString m_name;
    QString m_pbEnhancedType;
    QList<int> m_enhancedType;
    bool m_isSelected;
    QVariant m_viewCount;
    QString m_checkCountText;
};

Q_DECLARE_METATYPE(XrayImage)

#endif // XRAYIMAGE_H
