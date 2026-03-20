#include "qualitycontrolhistoryservice.h"

QualityControlHistoryService::QualityControlHistoryService(QObject *parent)
    : QualityControlServiceBase(parent)
{
}

void QualityControlHistoryService::startCheck()
{
    // TODO: replace with history-specific request
    emit statusMessage(QStringLiteral("history start check"));
}

void QualityControlHistoryService::pass()
{
    // TODO: replace with history-specific request
    emit statusMessage(QStringLiteral("history pass"));
}
