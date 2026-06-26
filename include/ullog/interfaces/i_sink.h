#pragma once

#include <ullog/severity.h>

#include <cstdint>

namespace ullog {

struct ISink {
    explicit ISink(Severity accepted = Severity::All);
    virtual ~ISink() = default;

    virtual void submit(Severity severity, const char* data, uint16_t len);
    virtual void flush() = 0;

protected:
    virtual void write(const char* data, uint16_t len) = 0;

private:
    [[nodiscard]] bool accepts(Severity severity) const;
    Severity accepted_;
};

}  // namespace ullog
