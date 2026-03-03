# argon2.pri - spdlog 日志库配置
ARGON2_PATH = $$PWD
INCLUDEPATH += \
    $$ARGON2_PATH/include \
    $$ARGON2_PATH/src

SOURCES += \
    $$ARGON2_PATH/src/argon2.c \
    $$ARGON2_PATH/src/core.c \
    $$ARGON2_PATH/src/blake2/blake2b.c \
    $$ARGON2_PATH/src/thread.c \
    $$ARGON2_PATH/src/encoding.c \
    $$ARGON2_PATH/src/ref.c

HEADERS += \
    $$ARGON2_PATH/include/argon2.h
