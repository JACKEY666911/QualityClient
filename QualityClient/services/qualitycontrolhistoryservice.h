#pragma once

#include "services/qualitycontrolservicebase.h"

class QualityControlHistoryService : public QualityControlServiceBase
{
    Q_OBJECT
public:
    explicit QualityControlHistoryService(QObject *parent = nullptr);

public slots:
    void startCheck() override;
    void pass() override;
};
