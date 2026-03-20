#include "qualitycontrolcontroller.h"

QualityControlController::QualityControlController(QObject *parent)
    : QObject(parent)
{
}

void QualityControlController::setAvailableTypes(const QSet<int> &types)
{
    emit availableTypesChanged(types);
}

void QualityControlController::setMainImage(const QPixmap &pixmap)
{
    emit mainImageChanged(pixmap);
}

void QualityControlController::setAuxImage(const QPixmap &pixmap)
{
    emit auxImageChanged(pixmap);
}

void QualityControlController::setJudgeResultText(const QString &text)
{
    emit judgeResultChanged(text);
}

void QualityControlController::setFreshnessText(const QString &text)
{
    emit freshnessChanged(text);
}

void QualityControlController::setChannelText(const QString &text)
{
    emit channelChanged(text);
}

void QualityControlController::setTimeText(const QString &text)
{
    emit timeTextChanged(text);
}
