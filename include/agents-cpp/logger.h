#pragma once

#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace agents {

/**
 * @brief Logger utility class that wraps spdlog functionality
 */
class Logger {
public:
    enum class Level {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        CRITICAL,
        OFF
    };

    /**
     * @brief Initialize the logger
     * @param level The log level
     */
    static void init(Level level = Level::INFO);

    /**
     * @brief Set the log level
     * @param level The log level
     */
    static void setLevel(Level level);

    /**
     * @brief Convert Level enum to spdlog level
     */
    static spdlog::level::level_enum toSpdlogLevel(Level level);

    /**
     * @brief Log a message at trace level
     */
    template<typename... Args>
    static void trace(const char* fmt, const Args&... args) {
        spdlog::trace(fmt, args...);
    }

    /**
     * @brief Log a message at debug level
     */
    template<typename... Args>
    static void debug(const char* fmt, const Args&... args) {
        spdlog::debug(fmt, args...);
    }

    /**
     * @brief Log a message at info level
     */
    template<typename... Args>
    static void info(const char* fmt, const Args&... args) {
        spdlog::info(fmt, args...);
    }

    /**
     * @brief Log a message at warn level
     */
    template<typename... Args>
    static void warn(const char* fmt, const Args&... args) {
        spdlog::warn(fmt, args...);
    }

    /**
     * @brief Log a message at error level
     */
    template<typename... Args>
    static void error(const char* fmt, const Args&... args) {
        spdlog::error(fmt, args...);
    }

    /**
     * @brief Log a message at critical level
     */
    template<typename... Args>
    static void critical(const char* fmt, const Args&... args) {
        spdlog::critical(fmt, args...);
    }

private:
    static std::shared_ptr<spdlog::logger> s_logger;
};

} // namespace agents 