#pragma once

#include <cstdint>
#include <type_traits>

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

constexpr Severity operator&(Severity lhs, Severity rhs) {
    return static_cast<Severity>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

constexpr Severity operator|(Severity lhs, Severity rhs) {
    return static_cast<Severity>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

template <typename... S>
constexpr Severity allExcept(S... excluded) {
    static_assert((std::is_same_v<S, Severity> && ...), "ALL_EXCEPT arguments must be ullog::Severity");
    const auto excludedMask = static_cast<uint8_t>((static_cast<uint8_t>(Severity::None) | ... | static_cast<uint8_t>(excluded)));
    return static_cast<Severity>(static_cast<uint8_t>(Severity::All) & ~excludedMask);
}

}  // namespace ullog

#define ALL_EXCEPT(...) ::ullog::allExcept(__VA_ARGS__)
