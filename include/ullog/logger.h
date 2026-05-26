#pragma once

#include "interfaces/i_clock.h"
#include "interfaces/i_sink.h"
#include "record.h"

namespace ullog {

class ILogger {
public:
    ILogger() = default;

    ILogger(const ILogger&) = delete;
    ILogger& operator=(const ILogger&) = delete;
    ILogger(ILogger&&) = delete;
    ILogger& operator=(ILogger&&) = delete;

    virtual ~ILogger() = default;

    static ILogger& get() {
        return *instance;
    }

    virtual Record createRecord(int line, const char* fileName, uint16_t fileNameLen,
                                const char* func, uint16_t funcLen) = 0;

protected:
    static ILogger* instance;
};

template <size_t SIZE>
class Logger : public ILogger {
    static_assert(SIZE >= Record::MIN_HEADER_SIZE * 3, "Logger buffer SIZE is too small to fit the log message");

public:
    Logger(IClock& clock, ISink& sink) : clock_(clock), sink_(sink) {
        instance = this;
    }

    ~Logger() override { instance = nullptr; }

    Record createRecord(int line, const char* fileName, uint16_t fileNameLen,
                        const char* func, uint16_t funcLen) override {
        return Record(buffer_, SIZE + MAX_TOKEN, line, fileName, fileNameLen, func, funcLen,
                      clock_.nowUs(), sink_);
    }

private:
    IClock& clock_;
    ISink& sink_;

    char buffer_[SIZE + MAX_TOKEN] = {};
};

}  // namespace ullog

#define LOG                                                                                                \
    ::ullog::ILogger::get().createRecord(__LINE__, __FILE_NAME__, sizeof(__FILE_NAME__) - 1, __FUNCTION__, \
                                         sizeof(__FUNCTION__) - 1)
