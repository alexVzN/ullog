#pragma once

#include "logger.h"
#include "severity.h"

// Call once at namespace scope, in a header included by every TU using LOGx.
// Hides a Meyers singleton inside ullog::detail — first LOG access constructs
// the Logger with the references you passed.
#define ULLOG_CREATE_DEFAULT(SIZE, MAX_SEV, CLOCK, SINK)                     \
    namespace ullog::detail {                                                \
    inline auto& activeLogger() {                                            \
        static ::ullog::Logger<(SIZE), (MAX_SEV)> instance((CLOCK), (SINK)); \
        return instance;                                                     \
    }                                                                        \
    }

#define ULLOG_DEFAULT (::ullog::detail::activeLogger())

// Helper macros (ULLOG_RECORD / ULLOG_RECORD_IF) — pulled in after ULLOG_DEFAULT
// so the dependency reads top-to-bottom.
#include "log_service_macros.h"

#define LOGD ULLOG_RECORD_IF(::ullog::Severity::Debug)
#define LOGI ULLOG_RECORD_IF(::ullog::Severity::Info)
#define LOGW ULLOG_RECORD_IF(::ullog::Severity::Warn)
#define LOGE ULLOG_RECORD_IF(::ullog::Severity::Error)
#define LOGF ULLOG_RECORD_IF(::ullog::Severity::Fatal)
