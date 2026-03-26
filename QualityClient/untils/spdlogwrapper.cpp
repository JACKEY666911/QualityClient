#include "spdlogwrapper.h"

#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <QDir>
#include <QFileInfo>
#include <QMessageLogContext>
#include <QString>
#include <QThread>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "services/settingsservice.h"

namespace {
std::string toStdString(const QString &value, const std::string &fallback)
{
    const QString trimmed = value.trimmed();
    return trimmed.isEmpty() ? fallback : trimmed.toStdString();
}

void logMessageHandler(QtMsgType type, const QMessageLogContext &context,
                       const QString &msg) {
    const char *category =
        (context.category != nullptr) ? context.category : "default";
    const std::string finalMsg = "[" + std::string(category) + "] " + msg.toStdString();
    static std::shared_ptr<spdlog::logger> s_qdebugLogger;
    if (s_qdebugLogger == nullptr) {
        s_qdebugLogger = spdlog::get("qdebug");
        if (s_qdebugLogger == nullptr) {
            return;
        }
    }

    switch (type) {
    case QtDebugMsg:
        s_qdebugLogger->debug(finalMsg);
        break;
    case QtInfoMsg:
        s_qdebugLogger->info(finalMsg);
        break;
    case QtWarningMsg:
        s_qdebugLogger->warn(finalMsg);
        break;
    case QtCriticalMsg:
        s_qdebugLogger->error(finalMsg);
        break;
    case QtFatalMsg:
        s_qdebugLogger->critical(finalMsg);
        std::abort();
    }
}
} // namespace

namespace spdlogwrapper {

// NOLINTBEGIN(readability-identifier-naming)
static std::unordered_map<size_t, std::string> g_thread_names;
static std::shared_mutex g_name_mutex;
// NOLINTEND(readability-identifier-naming)

void initThreadName() {
    QThread *thread = QThread::currentThread();
    if (thread == nullptr) {
        return;
    }
    const QString objectName = thread->objectName();
    if (objectName.isEmpty()) {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(g_name_mutex);
    g_thread_names[spdlog::details::os::thread_id()] = objectName.toStdString();
}

// NOLINTNEXTLINE(readability-identifier-naming)
class thread_name_formatter_flag : public spdlog::custom_flag_formatter {
public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    void format(const spdlog::details::log_msg &msg, const std::tm &tm_time,
                spdlog::memory_buf_t &dest) override {
        std::string name;
        {
            std::shared_lock lock(g_name_mutex);
            auto it = g_thread_names.find(msg.thread_id);
            if (it != g_thread_names.end()) {
                name = it->second;
            }
        }
        if (name.empty()) {
            name = std::to_string(msg.thread_id);
        }
        dest.append(name.data(), name.data() + name.size());
    }
    std::unique_ptr<custom_flag_formatter> clone() const override {
        return std::make_unique<thread_name_formatter_flag>();
    }
};

// NOLINTNEXTLINE(readability-identifier-naming)
class elapsed_time_flag : public spdlog::custom_flag_formatter {
public:
    elapsed_time_flag() : start_time_{std::chrono::steady_clock::now()} {}
    // NOLINTNEXTLINE(readability-identifier-naming)
    void format(const spdlog::details::log_msg &msg, const std::tm &tm_time,
                spdlog::memory_buf_t &dest) override {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - start_time_);

        // 格式化时长，例如: 00:00:01.500 (时:分:秒.毫秒)
        int64_t ms = duration.count();
        int seconds = static_cast<int>(ms / 1000) % 60;
        int minutes = static_cast<int>((ms / (1000L * 60)) % 60);
        int hours = static_cast<int>(ms / (1000L * 60 * 60));
        int msec = static_cast<int>(ms % 1000);

        char buffer[32];
        // 格式化为 [H:M:S.ms]
        int n = std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d.%03d", hours,
                              minutes, seconds, msec);

        dest.append(buffer, buffer + n);
    }
    std::unique_ptr<custom_flag_formatter> clone() const override {
        return std::make_unique<elapsed_time_flag>();
    }

private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    std::chrono::steady_clock::time_point start_time_;
};

void initLogger() {
    SettingsService &settings = SettingsService::instance();
    const QString logPathSetting = settings.value(QStringLiteral("Log/FilePath"),
                                                  QStringLiteral("logs/qualityclient.log"))
                                       .toString();
    const std::size_t maxSize = static_cast<std::size_t>(
        settings.value(QStringLiteral("Log/MaxSize"), 10 * 1024 * 1024).toLongLong());
    const std::size_t maxCount = static_cast<std::size_t>(
        settings.value(QStringLiteral("Log/MaxFiles"), 5).toInt());

    QFileInfo logFileInfo(logPathSetting);
    QDir dir(logFileInfo.path());
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
    const std::string logPath =
        toStdString(logFileInfo.absoluteFilePath(), "logs/qualityclient.log");

    if (spdlog::get("qdebug") != nullptr) {
        return;
    }
    spdlog::init_thread_pool(8192, 1);
    auto formatterConsole = std::make_unique<spdlog::pattern_formatter>();
    formatterConsole->add_flag<thread_name_formatter_flag>('N');
    formatterConsole->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%N] %v");
    auto consoleSink =
        std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
    consoleSink->set_formatter(std::move(formatterConsole));
#if defined(QT_DEBUG)
    consoleSink->set_level(spdlog::level::trace);
#else
    consoleSink->set_level(spdlog::level::info);
#endif
    auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath, maxSize, maxCount);
    auto formatterFile = std::make_unique<spdlog::pattern_formatter>();
    formatterFile->add_flag<thread_name_formatter_flag>('N');
    formatterFile->add_flag<elapsed_time_flag>('r');
    formatterFile->set_pattern("[%Y-%m-%d %H:%M:%S.%e(%r)] [%l] [%N] %v");
    fileSink->set_formatter(std::move(formatterFile));
    fileSink->set_level(spdlog::level::debug);
    const auto logger = std::make_shared<spdlog::async_logger>(
        "qdebug", spdlog::sinks_init_list{consoleSink, fileSink},
        spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::trace);
    spdlog::register_logger(logger);
    spdlog::flush_on(spdlog::level::err);
    qInstallMessageHandler(logMessageHandler);
}
} // namespace spdlogwrapper
