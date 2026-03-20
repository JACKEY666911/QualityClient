TARGET = QualityClient
TEMPLATE = app
QT       += core gui network concurrent

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
    Models/LayerViewModel.cpp \
    Models/ApiRequests.cpp \
    Models/JsonHelper.cpp \
    Models/ImageDistributeInfo.cpp \
    Models/XrayImage.cpp \
    GradientLabel.cpp \
    LoginView.cpp \
    MainView.cpp \
    MixModeSelectView.cpp \
    QualityControlBaseView.cpp \
    QualityControlHistoryView.cpp \
    QualityControlView.cpp \
    main.cpp \
    MainWindow.cpp \
    services/httpclient.cpp \
    services/httpclientasync.cpp \
    services/messagequeue.cpp \
    services/settingsservice.cpp \
    services/qualitycontrolshortcutmanager.cpp \
    services/apiservice.cpp \
    services/qualitycontrolcontroller.cpp \
    services/qualitycontrolservicebase.cpp \
    services/qualitycontrolservice.cpp \
    services/qualitycontrolhistoryservice.cpp \
    widgets/AnnotationGraphicsView.cpp \
    widgets/ThumbnailWidget.cpp \
    widgets/XrayImageButton.cpp \
    widgets/CheckBoxDelegate.cpp \
    widgets/CheckComBoBox.cpp \
    widgets/LayerViewWidget.cpp \
    widgets/CollapsiblePanel.cpp \
    widgets/SwitchButton.cpp \
    widgets/flowlayout.cpp \
    widgets/moderndatetimepicker.cpp \
    widgets/paginationwidget.cpp

HEADERS += \
    Models/LayerViewModel.h \
    Models/JsonHelper.h \
    Models/ImageDistributeInfo.h \
    Models/ApiRequests.h \
    Models/XImageType.h \
    Models/XrayImage.h \
    GradientLabel.h \
    LoginView.h \
    MainView.h \
    MainWindow.h \
    MixModeSelectView.h \
    QualityControlBaseView.h \
    QualityControlHistoryView.h \
    QualityControlView.h \
    services/httpclient.h \
    services/httpclientasync.h \
    services/messagequeue.h \
    services/settingsservice.h \
    services/qualitycontrolshortcutmanager.h \
    services/apiservice.h \
    services/qualitycontrolcontroller.h \
    services/qualitycontrolservicebase.h \
    services/qualitycontrolservice.h \
    services/qualitycontrolhistoryservice.h \
    widgets/AnnotationGraphicsView.h \
    widgets/ThumbnailWidget.h \
    widgets/XrayImageButton.h \
    widgets/CheckBoxDelegate.h \
    widgets/CheckComBoBox.h \
    widgets/LayerViewWidget.h \
    widgets/CollapsiblePanel.h \
    widgets/SwitchButton.h \
    widgets/flowlayout.h \
    widgets/moderndatetimepicker.h \
    widgets/paginationwidget.h

FORMS += \
    LoginView.ui \
    MainView.ui \
    MainWindow.ui \
    MixModeSelectView.ui

include(libraries/spdlog/spdlog.pri)
include(libraries/argon2/argon2.pri)
include(libraries/singleapplication/singleapplication.pri)
include(libraries/qhotkey/qhotkey.pri)


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rc.qrc

DISTFILES +=

