#include "qualitycontrolservicebase.h"
QualityControlServiceBase::QualityControlServiceBase(QObject *parent)
    : QObject(parent)
    , m_api(&ApiService::instance())
{
}

QualityControlServiceBase::~QualityControlServiceBase() = default;

void QualityControlServiceBase::startCheck()
{
    emit statusMessage(QStringLiteral("start check"));
}

void QualityControlServiceBase::pass()
{
    emit statusMessage(QStringLiteral("pass"));
}

void QualityControlServiceBase::openPersonDetail()
{
    emit statusMessage(QStringLiteral("open person detail"));
}

void QualityControlServiceBase::switchXrayType(int type, bool selected)
{
    Q_UNUSED(type);
    Q_UNUSED(selected);
}
