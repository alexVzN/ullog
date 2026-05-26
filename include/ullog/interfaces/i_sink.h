#pragma once

#include <cstdint>

namespace ullog {

struct ISink {
    virtual ~ISink() = default;
    virtual void write(const char* data, uint16_t len) = 0;
    virtual void flush() = 0;
};

}  // namespace ullog
