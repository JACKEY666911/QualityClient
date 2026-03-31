#include <QApplication>
#include <QPixmapCache>
#include <QThread>

#include "MainView.h"
#include "logging/logcategories.h"
#include "services/settingsservice.h"
#include "untils/spdlogwrapper.h"
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  QThread::currentThread()->setObjectName(QStringLiteral("MainThread"));
  spdlogwrapper::initThreadName();
  spdlogwrapper::initLogger();
  QPixmapCache::setCacheLimit(50 * 1024);
  qCInfo(lcApp) << "QualityClient started";
  MainView w;
  w.show();
  return a.exec();
}
