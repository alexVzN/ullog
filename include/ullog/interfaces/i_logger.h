#pragma once

#include <cstdint>

#include "../record.h"
#include "../severity.h"

namespace ullog {

struct ILogger {
    virtual ~ILogger() = default;

    virtual Record createRecord(Severity severity, int line, const char* fileName, uint16_t fileNameLen,
                                const char* func, uint16_t funcLen) = 0;
};

}  // namespace ullog
