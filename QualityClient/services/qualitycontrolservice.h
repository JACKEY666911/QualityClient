#pragma once

#include "services/qualitycontrolservicebase.h"

class QualityControlService : public QualityControlServiceBase
{
    Q_OBJECT
public:
    explicit QualityControlService(QObject *parent = nullptr);

public slots:
    void startCheck() override;
    void pass() override;
};
