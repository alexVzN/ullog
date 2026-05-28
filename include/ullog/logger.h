#pragma once

#include "interfaces/i_clock.h"
#include "interfaces/i_sink.h"
#include "record.h"
#include "severity.h"

namespace ullog {

template <size_t SIZE, Severity MAX_SEV = Severity::All>
class Logger {
    static_assert(SIZE >= Record::MIN_HEADER_SIZE * 3, "Logger buffer SIZE is too small to fit the log message");

public:
    static constexpr Severity max_severity = MAX_SEV;

    Logger(IClock& clock, ISink& sink) : clock_(clock), sink_(sink) {}

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    Record createRecord(Severity severity, int line, const char* fileName, uint16_t fileNameLen, const char* func,
                        uint16_t funcLen) {
        return Record(buffer_, SIZE + MAX_TOKEN, severity, line, fileName, fileNameLen, func, funcLen, clock_.nowUs(),
                      sink_);
    }

private:
    IClock& clock_;
    ISink& sink_;

    char buffer_[SIZE + MAX_TOKEN] = {};
};

}  // namespace ullog
