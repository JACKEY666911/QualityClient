#include "LoginView.h"
#include "MainView.h"
#include "untils/spdlogwrapper.h"
#include "logging/logcategories.h"

#include <QApplication>
#include <QThread>
#include "services/settingsservice.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QThread::currentThread()->setObjectName(QStringLiteral("MainThread"));
    spdlogwrapper::initThreadName();
    spdlogwrapper::initLogger();
    qCInfo(lcApp) << "QualityClient started";
    MainView w;
    w.show();
    return a.exec();
}
