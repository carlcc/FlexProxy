
#include "FlexCommon/Log.h"
#include <iostream>

//#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
//#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace FS {

static std::shared_ptr<spdlog::logger> gLogger = nullptr;
static LogLevel gLogLevel { LogLevel::kInfo };

extern spdlog::logger* GetLogger()
{
    return gLogger.get();
}

static void SetLogLevel(LogLevel logLevel)
{
    if (gLogger != nullptr) {
        if (logLevel < LogLevel::kError || logLevel >= LogLevel::kCount) {
            std::cerr << "Unknown log level! use default instead" << std::endl;
            logLevel = LogLevel::kInfo;
        }

        spdlog::level::level_enum level;
        switch (logLevel) {
        case LogLevel::kDebug:
            level = spdlog::level::debug;
            break;
        case LogLevel::kInfo:
            level = spdlog::level::info;
            break;
        case LogLevel::kWarn:
            level = spdlog::level::warn;
            break;
        case LogLevel::kError:
            level = spdlog::level::err;
            break;
        default:
            abort();
        }
        gLogLevel = logLevel;
        gLogger->set_level(level);
        gLogger->flush_on(level);
    }
}

extern bool LoggerInit(const char* logFilePath, LogLevel logLevel)
{
    if (nullptr != logFilePath) {
        gLogger = spdlog::daily_logger_mt("file", logFilePath);
    } else {
        gLogger = spdlog::stdout_color_mt("con");
    }
    if (gLogger == nullptr) {
        return false;
    }

    SetLogLevel(logLevel);
    spdlog::set_pattern("%^[%Y%m%d %H:%M:%S.%f][%l]%$%v");
    return true;
}

extern void LoggerClean()
{
    gLogger = nullptr;
}

}