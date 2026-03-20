#include "qualitycontrolservice.h"

QualityControlService::QualityControlService(QObject *parent)
    : QualityControlServiceBase(parent)
{
}

void QualityControlService::startCheck()
{
    // TODO: replace with real request
    emit statusMessage(QStringLiteral("quality control start check"));
}

void QualityControlService::pass()
{
    // TODO: replace with real request
    emit statusMessage(QStringLiteral("quality control pass"));
}
