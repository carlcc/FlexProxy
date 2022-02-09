#pragma once

#ifdef __GNUC__
#pragma GCC diagnostic push
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
#include "spdlog/spdlog.h"
#pragma GCC diagnostic pop
#else
#include "spdlog/spdlog.h"
#endif

namespace FS {

enum class LogLevel {
    kError,
    kWarn,
    kInfo,
    kDebug,
    kCount,
};

extern spdlog::logger* GetLogger();

extern bool LoggerInit(const char* logFilePath, LogLevel logLevel);

extern void LoggerClean();

}

#define L_PRINT(fmt, ...)                            \
    do {                                             \
        ::FS::GetLogger()->info(fmt, ##__VA_ARGS__); \
    } while (false)

#define L_DEBUG(fmt, ...)                                                            \
    do {                                                                             \
        ::FS::GetLogger()->debug("({}:{}: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (false)
#define L_INFO(fmt, ...)                                                            \
    do {                                                                            \
        ::FS::GetLogger()->info("({}:{}: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (false)
#define L_WARNING(fmt, ...)                                                         \
    do {                                                                            \
        ::FS::GetLogger()->warn("({}:{}: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (false)
#define L_ERROR(fmt, ...)                                                            \
    do {                                                                             \
        ::FS::GetLogger()->error("({}:{}: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (false)

#define L_ASSERT(condition, fmt, ...)    \
    do {                                 \
        if (!(condition)) {              \
            L_ERROR(fmt, ##__VA_ARGS__); \
            abort();                     \
        }                                \
    } while (false)

#define L_INFO_EVERY_N(N, fmt, ...)     \
    do {                                \
        static int count = 0;           \
        if (++count >= N) {             \
            count = 0;                  \
            L_INFO(fmt, ##__VA_ARGS__); \
        }                               \
    } while (false)

#define SCOPE_EXIT_INFO(fmt, ...) std::shared_ptr<int> _SCOPE_EXIT_INFOER_((int*)1000, [&](int*) { L_INFO(fmt, ##__VA_ARGS__); })