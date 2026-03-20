#pragma once

#include <QObject>
#include <QSet>
#include <QPixmap>

#include "services/apiservice.h"
class QualityControlServiceBase : public QObject
{
    Q_OBJECT
public:
    explicit QualityControlServiceBase(QObject *parent = nullptr);
    ~QualityControlServiceBase() override;

public slots:
    virtual void startCheck();
    virtual void pass();
    virtual void openPersonDetail();
    virtual void switchXrayType(int type, bool selected);

signals:
    void errorOccurred(const QString &message);
    void statusMessage(const QString &message);
    void availableTypesReceived(const QSet<int> &types);
    void mainImageReceived(const QPixmap &pixmap);
    void auxImageReceived(const QPixmap &pixmap);
    void judgeResultReceived(const QString &text);
    void freshnessReceived(const QString &text);
    void channelReceived(const QString &text);
    void timeTextReceived(const QString &text);

protected:
    ApiService *m_api;
};
