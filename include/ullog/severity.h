#pragma once

#include <cstdint>

namespace ullog {

enum class Severity : uint8_t {
    None = 0x00,
    Fatal = 0x01,
    Error = 0x02,
    Warn = 0x04,
    Info = 0x08,
    Debug = 0x10,

    All = 0xFF,
};

constexpr char severityChar(Severity s) {
    switch (s) {
        case Severity::Fatal:
            return 'F';
        case Severity::Error:
            return 'E';
        case Severity::Warn:
            return 'W';
        case Severity::Info:
            return 'I';
        case Severity::Debug:
            return 'D';
        default:
            return '?';
    }
}

}  // namespace ullog
