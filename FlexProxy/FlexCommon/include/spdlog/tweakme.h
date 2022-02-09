//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

///////////////////////////////////////////////////////////////////////////////
//
// Edit this file to squeeze more performance, and to customize supported
// features
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Under Linux, the much faster CLOCK_REALTIME_COARSE clock can be used.
// This clock is less accurate - can be off by dozens of millis - depending on
// the kernel HZ.
// Uncomment to use it instead of the regular clock.
//
// #define SPDLOG_CLOCK_COARSE
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment if date/time logging is not needed and never appear in the log
// pattern.
// This will prevent spdlog from querying the clock on each log call.
//
// WARNING: If the log pattern contains any date/time while this flag is on, the
// result is undefined.
//          You must set new pattern(spdlog::set_pattern(..") without any
//          date/time in it
//
// #define SPDLOG_NO_DATETIME
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment if thread id logging is not needed (i.e. no %t in the log pattern).
// This will prevent spdlog from querying the thread id on each log call.
//
// WARNING: If the log pattern contains thread id (i.e, %t) while this flag is
// on, the result is undefined.
//
// #define SPDLOG_NO_THREAD_ID
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment to prevent spdlog from caching thread ids in thread local storage.
// By default spdlog saves thread ids in tls to gain a few micros for each call.
//
// WARNING: if your program forks, UNCOMMENT this flag to prevent undefined
// thread ids in the children logs.
//
// #define SPDLOG_DISABLE_TID_CACHING
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment if logger name logging is not needed.
// This will prevent spdlog from copying the logger name on each log call.
//
// #define SPDLOG_NO_NAME
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment to enable the SPDLOG_DEBUG/SPDLOG_TRACE macros.
//
// #define SPDLOG_DEBUG_ON
// #define SPDLOG_TRACE_ON
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment to avoid spdlog's usage of atomic log levels
// Use only if your code never modifies a logger's log levels concurrently by
// different threads.
//
// #define SPDLOG_NO_ATOMIC_LEVELS
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment to enable usage of wchar_t for file names on Windows.
//
// #define SPDLOG_WCHAR_FILENAMES
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment to override default eol ("\n" or "\r\n" under Linux/Windows)
//
// #define SPDLOG_EOL ";-)\n"
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment to use your own copy of the fmt library instead of spdlog's copy.
// In this case spdlog will try to include <fmt/format.h> so set your -I flag
// accordingly.
//
// #define SPDLOG_FMT_EXTERNAL
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment to enable wchar_t support (convert to utf8)
//
// #define SPDLOG_WCHAR_TO_UTF8_SUPPORT
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment to prevent child processes from inheriting log file descriptors
//
// #define SPDLOG_PREVENT_CHILD_FD
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment if your compiler doesn't support the "final" keyword.
// The final keyword allows more optimizations in release
// mode with recent compilers. See GCC's documentation for -Wsuggest-final-types
// for instance.
//
// #define SPDLOG_NO_FINAL
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment to enable message counting feature.
// Use the %i in the logger pattern to display log message sequence id.
//
// #define SPDLOG_ENABLE_MESSAGE_COUNTER
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Uncomment to customize level names (e.g. "MT TRACE")
//
#define SPDLOG_LEVEL_NAMES { "TRACE", "DEBUG", "INFO", "WARNING","ERR", "CRITICAL", "OFF" }
///////////////////////////////////////////////////////////////////////////////
