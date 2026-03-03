#ifndef SETTINGSSERVICE_H
#define SETTINGSSERVICE_H

#include <QObject>

class settingsservice : public QObject
{
    Q_OBJECT
public:
    explicit settingsservice(QObject *parent = nullptr);

signals:

};

#endif // SETTINGSSERVICE_H
