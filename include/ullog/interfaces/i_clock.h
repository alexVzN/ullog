#pragma once

#include <cstdint>

namespace ullog {

struct IClock {
    virtual ~IClock() = default;
    virtual uint32_t nowUs() = 0;
};

}  // namespace ullog
