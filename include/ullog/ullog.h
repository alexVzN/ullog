#pragma once

#include "interfaces/i_logger.h"
#include "logger.h"
#include "severity.h"

#ifndef ULLOG_MAX_SEVERITY
#define ULLOG_MAX_SEVERITY ::ullog::Severity::All
#endif

namespace ullog::detail {

inline constexpr Severity kMaxSeverity = ULLOG_MAX_SEVERITY;

ILogger& activeLogger();

}  // namespace ullog::detail

#define ULLOG_CREATE_DEFAULT(SIZE, MAX_SEV, CLOCK, SINK)                     \
    namespace ullog::detail {                                                \
    ::ullog::ILogger& activeLogger() {                                       \
        static ::ullog::Logger<(SIZE), (MAX_SEV)> instance((CLOCK), (SINK)); \
        return instance;                                                     \
    }                                                                        \
    }

#define ULLOG_DEFAULT (::ullog::detail::activeLogger())

#include "log_service_macros.h"

#define LOGD ULLOG_RECORD_IF(::ullog::Severity::Debug)
#define LOGI ULLOG_RECORD_IF(::ullog::Severity::Info)
#define LOGW ULLOG_RECORD_IF(::ullog::Severity::Warn)
#define LOGE ULLOG_RECORD_IF(::ullog::Severity::Error)
#define LOGF ULLOG_RECORD_IF(::ullog::Severity::Fatal)
