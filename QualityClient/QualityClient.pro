TARGET = QualityClient
TEMPLATE = app
QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 debug
#include(modules/fontmanager/fontmanager.pri)
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



DEPTH = ../qamqp
message("DEPTH: $$DEPTH")  # 打印 DEPTH 路径
include($${DEPTH}/qamqp.pri)
INCLUDEPATH += $${QAMQP_INCLUDEPATH}
LIBS += -L$${DEPTH}/src $${QAMQP_LIBS}
message("conpleteLIBS: $$LIBS")

# 查看qmake处理后的实际链接参数
QMAKE_LFLAGS ~= s/-L/\\n-L/g
message("dadadddeeq:\n$$QMAKE_LFLAGS")
INCLUDEPATH += $$PWD/libraries
SOURCES += \
    GradientLabel.cpp \
    LoginView.cpp \
    MainView.cpp \
    MixModeSelectView.cpp \
    main.cpp \
    MainWindow.cpp \
    services/httpclient.cpp \
    services/messagequeue.cpp \
    services/settingsservice.cpp \
    widgets/CheckBoxDelegate.cpp \
    widgets/CheckComBoBox.cpp

HEADERS += \
    GradientLabel.h \
    LoginView.h \
    MainView.h \
    MainWindow.h \
    MixModeSelectView.h \
    services/httpclient.h \
    services/messagequeue.h \
    services/settingsservice.h \
    widgets/CheckBoxDelegate.h \
    widgets/CheckComBoBox.h

FORMS += \
    LoginView.ui \
    MainView.ui \
    MainWindow.ui \
    MixModeSelectView.ui

include(libraries/spdlog/spdlog.pri)
include(libraries/argon2/argon2.pri)
include(libraries/singleapplication/singleapplication.pri)



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rc.qrc

