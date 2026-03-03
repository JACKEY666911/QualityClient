# spdlog.pri - spdlog 日志库配置


HEADERS += \
    $$PWD/include/spdlog/*.h \
    $$PWD/include/spdlog/details/*.h \
    $$PWD/include/spdlog/sinks/*.h \
    $$PWD/include/spdlog/fmt/*.h \
    $$PWD/include/spdlog/fmt/bundled/*.h

# 头文件路径
INCLUDEPATH += $$PWD/include/spdlog

# 如果使用的是预编译的库（可选）
# LIBS += -L$$PWD/lib -lspdlog

# 定义宏（可选）
DEFINES += SPDLOG_FMT_EXTERNAL  # 如果使用外部fmt库
# DEFINES += SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE  # 设置日志级别

# 如果是Debug版本，可以启用额外的调试功能
CONFIG(debug, debug|release) {
    DEFINES += SPDLOG_DEBUG_ON
    # DEFINES += SPDLOG_TRACE_ON
} else {
    DEFINES += SPDLOG_DEBUG_OFF
    # DEFINES += SPDLOG_TRACE_OFF
}

# 跨平台支持
win32 {
    # Windows特定的设置
    INCLUDEPATH += $$PWD/include
    # LIBS += -lUser32 -lAdvapi32  # spdlog在某些Windows功能需要
}

unix {
    # Linux/Unix特定的设置
    INCLUDEPATH += $$PWD/include
}

macx {
    # macOS特定的设置
    INCLUDEPATH += $$PWD/include
}

# 打印包含路径（用于调试）
message("spdlog include path: $$PWD/include")
