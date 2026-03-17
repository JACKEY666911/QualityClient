#ifndef XRAYIMAGE_H
#define XRAYIMAGE_H

#include <QObject>
#include <QString>
#include <QList>

class XrayImage : public QObject
{
    Q_OBJECT
public:
    explicit XrayImage(QObject *parent = nullptr);

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

signals:
    void dataChanged();
    void selectionChanged(bool selected);
    void nameChanged(const QString &name);

private:
    void updateNameFromEnhancedType();
    void updateNameFromPbEnhancedType(const QString &value);
    void updateNameWithViewCount(const QString &fallback);

    QString m_fullXrayImageUrl;
    QString m_mainXrayImageUrl;
    QString m_assistXrayImageUrl;
    int m_qualityResult;
    QString m_name;
    QString m_pbEnhancedType;
    QList<int> m_enhancedType;
    bool m_isSelected;
    qint64 m_viewCount;
    QString m_checkCountText;
};

#endif // XRAYIMAGE_H
