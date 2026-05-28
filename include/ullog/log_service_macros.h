#pragma once

// Internal helpers for ullog.h. NOT self-contained — the macros below
// textually reference ULLOG_DEFAULT, which ullog.h defines before pulling
// this header in. Do not include this file on its own.

#include <cstdint>
#include <type_traits>

#include "record.h"

namespace ullog::detail {

// Drops a `<< a << b` chain to void without evaluating it: operator& binds
// tighter than ?: but looser than <<, so the chain ends up nested inside the
// ternary's dead branch.
struct Voidify {
    void operator&(const Record&) {}
};

}  // namespace ullog::detail

#define ULLOG_RECORD(sev)                                                                               \
    ULLOG_DEFAULT.createRecord((sev), __LINE__, __FILE_NAME__, sizeof(__FILE_NAME__) - 1, __FUNCTION__, \
                               sizeof(__FUNCTION__) - 1)

#define ULLOG_RECORD_IF(sev)                                                               \
    (static_cast<uint8_t>(sev) >                                                           \
     static_cast<uint8_t>(std::remove_reference_t<decltype(ULLOG_DEFAULT)>::max_severity)) \
        ? (void)0                                                                          \
        : ::ullog::detail::Voidify() & ULLOG_RECORD(sev)
