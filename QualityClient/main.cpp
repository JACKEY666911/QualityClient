#include "LoginView.h"
#include "MainView.h"

#include <QApplication>
#include "services/settingsservice.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainView w;
    w.show();
    return a.exec();
}
