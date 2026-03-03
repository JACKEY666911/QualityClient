# 1. 配置模块依赖（字体管理必加 core，操作控件加 widgets）
QT += core widgets
# 2. 引入模块源文件和头文件（$$PWD 表示当前 .pri 所在目录，无需改路径）
SOURCES += \
           $$PWD/FontLoader.cpp \
           $$PWD/FontManager.cpp \

HEADERS += \
           $$PWD/FontConfig.h \
           $$PWD/FontLoader.h \
           $$PWD/FontManager.h \


# 3. 引入模块资源文件（若有，路径对应自己的 qrc）
RESOURCES +=
    $$PWD/fonts.qrc
# 5. 暴露头文件路径，方便主项目/其他模块引用
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
