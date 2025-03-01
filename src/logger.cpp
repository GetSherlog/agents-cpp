#include "agents-cpp/logger.h"

namespace agents {

std::shared_ptr<spdlog::logger> Logger::s_logger;

void Logger::init(Level level) {
    s_logger = spdlog::stdout_color_mt("agents-cpp");
    setLevel(level);
    
    // Set pattern: [time] [level] message
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
}

void Logger::setLevel(Level level) {
    spdlog::set_level(toSpdlogLevel(level));
}

spdlog::level::level_enum Logger::toSpdlogLevel(Level level) {
    switch (level) {
        case Level::TRACE:    return spdlog::level::trace;
        case Level::DEBUG:    return spdlog::level::debug;
        case Level::INFO:     return spdlog::level::info;
        case Level::WARN:     return spdlog::level::warn;
        case Level::ERROR:    return spdlog::level::err;
        case Level::CRITICAL: return spdlog::level::critical;
        case Level::OFF:      return spdlog::level::off;
        default:              return spdlog::level::info;
    }
}

} // namespace agents 