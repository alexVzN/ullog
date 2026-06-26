#include "ullog/interfaces/i_sink.h"

namespace ullog {

ISink::ISink(Severity accepted)
    : accepted_(accepted) {}

bool ISink::accepts(Severity severity) const {
    return (accepted_ & severity) == severity;
}

void ISink::submit(Severity severity, const char* data, uint16_t len) {
    if (accepts(severity))
        write(data, len);
}

} // namespace ullog
