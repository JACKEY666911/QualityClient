# spdlog header-only configuration

# Keep only include root; spdlog will use bundled fmt in header-only mode.
INCLUDEPATH += $$PWD/include

# Header-only mode is auto-selected when SPDLOG_COMPILED_LIB is not defined.
# Do NOT define SPDLOG_HEADER_ONLY here to avoid macro redefinition warnings.

CONFIG(debug, debug|release) {
    DEFINES += SPDLOG_DEBUG_ON
} else {
    DEFINES += SPDLOG_DEBUG_OFF
}

message("spdlog(header-only) include path: $$PWD/include")
