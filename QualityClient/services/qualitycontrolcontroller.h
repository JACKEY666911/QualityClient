#pragma once

#include <QObject>
#include <QSet>
#include <QPixmap>

class QualityControlController : public QObject
{
    Q_OBJECT
public:
    explicit QualityControlController(QObject *parent = nullptr);

public slots:
    void setAvailableTypes(const QSet<int> &types);
    void setMainImage(const QPixmap &pixmap);
    void setAuxImage(const QPixmap &pixmap);
    void setJudgeResultText(const QString &text);
    void setFreshnessText(const QString &text);
    void setChannelText(const QString &text);
    void setTimeText(const QString &text);

signals:
    void availableTypesChanged(const QSet<int> &types);
    void mainImageChanged(const QPixmap &pixmap);
    void auxImageChanged(const QPixmap &pixmap);
    void judgeResultChanged(const QString &text);
    void freshnessChanged(const QString &text);
    void channelChanged(const QString &text);
    void timeTextChanged(const QString &text);
};
