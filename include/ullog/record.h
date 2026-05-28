#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <type_traits>

#include "converter_to_cstring.h"
#include "interfaces/i_sink.h"
#include "severity.h"

namespace ullog {

class Record {
public:
    static constexpr uint8_t META_WIDTH = 50;
    static constexpr uint8_t TIME_S_WIDTH = 4;
    static constexpr uint8_t TIME_US_WIDTH = 6;
    static constexpr uint8_t SEVERITY_WIDTH = 1;
    // '[' + TIME_S_WIDTH + '.' + TIME_US_WIDTH + ']' + '[' + SEVERITY_WIDTH + ']' + '[' + META_WIDTH + ']' + ' '
    static constexpr uint16_t MIN_HEADER_SIZE =
        1 + TIME_S_WIDTH + 1 + TIME_US_WIDTH + 1 + 1 + SEVERITY_WIDTH + 1 + 1 + META_WIDTH + 1 + 1;

    Record(char* buffer, uint16_t bufferSize, Severity severity, int line, const char* fileName,
           uint16_t fileNameSize, const char* func, uint16_t funcNameSize, uint32_t timestamp_us, ISink& sink);

    ~Record();

    Record(const Record&) = delete;
    Record& operator=(const Record&) = delete;
    Record(Record&&) = delete;
    Record& operator=(Record&&) = delete;

    Record& operator<<(const char* value);
    Record& operator<<(bool value);
    Record& operator<<(float value);
    Record& operator<<(HexArg arg);
    Record& operator<<(BinArg arg);
    Record& operator<<(FloatArg arg);

    template <size_t N>
    Record& operator<<(const char (&str)[N]) {
        if (position_ > max_safe_position_)
            return *this;

        constexpr uint16_t len = N - 1;
        append(str, len);
        return *this;
    }

    template <typename T>
    Record& operator<<(const T* value) {
        if (position_ > max_safe_position_)
            return *this;

        position_ += detail::toCString(buffer_ + position_, buffer_ + buffer_size_,
                                       HexArg{static_cast<uint32_t>(reinterpret_cast<uintptr_t>(value)), 8});
        return *this;
    }

    template <typename T,
              typename = std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T> && !std::is_same_v<T, bool>>>
    Record& operator<<(T value) {
        if (position_ > max_safe_position_)
            return *this;

        position_ += detail::toCString(buffer_ + position_, buffer_ + buffer_size_, static_cast<int32_t>(value));
        return *this;
    }

    template <typename T,
              typename = std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T> && !std::is_same_v<T, bool>>,
              typename = void>
    Record& operator<<(T value) {
        if (position_ > max_safe_position_)
            return *this;

        position_ += detail::toCString(buffer_ + position_, buffer_ + buffer_size_, static_cast<uint32_t>(value));
        return *this;
    }

private:
    inline void append(char ch) {
        if (position_ < buffer_size_)
            buffer_[position_++] = ch;
    }

    inline void append(const char* str, uint16_t len) {
        uint16_t available = buffer_size_ - MAX_TOKEN - position_;
        uint16_t toCopy = std::min(len, available);
        memcpy(buffer_ + position_, str, toCopy);
        position_ += toCopy;
    }

    char* buffer_ = nullptr;
    uint16_t buffer_size_ = 0;
    uint16_t max_safe_position_;
    uint16_t position_ = 0;
    Severity severity_;
    ISink& sink_;
};

}  // namespace ullog