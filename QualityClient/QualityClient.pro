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

INCLUDEPATH += $$PWD/thirdparty/hikvision/include
LIBS += -L$$PWD/thirdparty/hikvision/lib \
        -lHCNetSDK \
        -lPlayCtrl \
        -lHCCore \
        -lws2_32 \

DEFINES += WIN32 _WINSOCKAPI_

# 查看qmake处理后的实际链接参数
QMAKE_LFLAGS ~= s/-L/\\n-L/g
message("dadadddeeq:\n$$QMAKE_LFLAGS")
INCLUDEPATH += $$PWD/libraries
SOURCES += \
    HistoryMainView.cpp \
    HistoryMainViewWidget.cpp \
    Models/LayerViewModel.cpp \
    PersonBaggageView.cpp \
    VideoPlaybackView.cpp \
    logging/logcategories.cpp \
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
    services/imageprovider.cpp \
    services/messagequeue.cpp \
    services/settingsservice.cpp \
    services/qualitycontrolshortcutmanager.cpp \
    services/apiservice.cpp \
    services/hikvisionsdkservice.cpp \
    services/qualitycontrolservicebase.cpp \
    services/qualitycontrolservice.cpp \
    services/qualitycontrolhistoryservice.cpp \
    untils/spdlogwrapper.cpp \
    untils/timecounter.cpp \
    widgets/AnnotationGraphicsView.cpp \
    widgets/QualityCardDelegate.cpp \
    widgets/ThumbnailWidget.cpp \
    widgets/TimeLineSlider.cpp \
    widgets/XrayImageButton.cpp \
    widgets/CheckBoxDelegate.cpp \
    widgets/CheckComBoBox.cpp \
    widgets/LayerViewWidget.cpp \
    widgets/CollapsiblePanel.cpp \
    widgets/SwitchButton.cpp \
    widgets/flowlayout.cpp \
    dialogs/appmessagedialog.cpp \
    widgets/moderndatetimepicker.cpp \
    widgets/paginationwidget.cpp \
    dialogs/toastnotification.cpp

HEADERS += \
    HistoryMainView.h \
    HistoryMainViewWidget.h \
    Models/GlobalEnums.h \
    Models/LayerViewModel.h \
    Models/QueryInfo.h \
    PersonBaggageView.h \
    VideoPlaybackView.h \
    logging/logcategories.h \
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
    services/imageprovider.h \
    services/messagequeue.h \
    services/settingsservice.h \
    services/qualitycontrolshortcutmanager.h \
    services/apiservice.h \
    services/hikvisionsdkservice.h \
    services/qualitycontrolservicebase.h \
    services/qualitycontrolservice.h \
    services/qualitycontrolhistoryservice.h \
    thirdparty/hikvision/include/DataType.h \
    thirdparty/hikvision/include/DecodeCardSdk.h \
    thirdparty/hikvision/include/HCNetSDK.h \
    thirdparty/hikvision/include/plaympeg4.h \
    untils/spdlogwrapper.h \
    untils/timecounter.h \
    widgets/AnnotationGraphicsView.h \
    widgets/QualityCardDelegate.h \
    widgets/ThumbnailWidget.h \
    widgets/TimeLineSlider.h \
    widgets/XrayImageButton.h \
    widgets/CheckBoxDelegate.h \
    widgets/CheckComBoBox.h \
    widgets/LayerViewWidget.h \
    widgets/CollapsiblePanel.h \
    widgets/SwitchButton.h \
    widgets/flowlayout.h \
    dialogs/appmessagedialog.h \
    widgets/moderndatetimepicker.h \
    widgets/paginationwidget.h \
    dialogs/toastnotification.h

FORMS += \
    HistoryMainView.ui \
    LoginView.ui \
    MainView.ui \
    MainWindow.ui \
    MixModeSelectView.ui \
    PersonBaggageView.ui \
    VideoPlaybackView.ui

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

DISTFILES += \
    thirdparty/hikvision/lib/AudioProcess.dll \
    thirdparty/hikvision/lib/AudioRender.dll \
    thirdparty/hikvision/lib/GdiPlus.lib \
    thirdparty/hikvision/lib/HCCore.dll \
    thirdparty/hikvision/lib/HCCore.lib \
    thirdparty/hikvision/lib/HCNetSDK.dll \
    thirdparty/hikvision/lib/HCNetSDK.lib \
    thirdparty/hikvision/lib/HCNetSDKCom/AnalyzeData.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/AudioIntercom.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/AudioProcess.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/AudioRender.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/HCAlarm.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/HCAlarm.lib \
    thirdparty/hikvision/lib/HCNetSDKCom/HCCoreDevCfg.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/HCDisplay.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/HCGeneralCfgMgr.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/HCGeneralCfgMgr.lib \
    thirdparty/hikvision/lib/HCNetSDKCom/HCIndustry.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/HCPlayBack.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/HCPreview.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/HCPreview.lib \
    thirdparty/hikvision/lib/HCNetSDKCom/HCVoiceTalk.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/OpenAL32.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/StreamTransClient.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/SystemTransform.dll \
    thirdparty/hikvision/lib/HCNetSDKCom/libiconv2.dll \
    thirdparty/hikvision/lib/HXVA.dll \
    thirdparty/hikvision/lib/HmMerge.dll \
    thirdparty/hikvision/lib/LocalXml.zip \
    thirdparty/hikvision/lib/MP_Render.dll \
    thirdparty/hikvision/lib/NPQos.dll \
    thirdparty/hikvision/lib/OpenAL32.dll \
    thirdparty/hikvision/lib/PlayCtrl.dll \
    thirdparty/hikvision/lib/PlayCtrl.lib \
    thirdparty/hikvision/lib/SuperRender.dll \
    thirdparty/hikvision/lib/YUVProcess.dll \
    thirdparty/hikvision/lib/gdiplus.dll \
    thirdparty/hikvision/lib/hlog.dll \
    thirdparty/hikvision/lib/hpr.dll \
    thirdparty/hikvision/lib/libcrypto-3-x64.dll \
    thirdparty/hikvision/lib/libssl-3-x64.dll \
    thirdparty/hikvision/lib/zlib1.dll

