#include "ullog/record.h"

#include <cstdio>
#include <cstring>

namespace ullog {

Record::Record(char* buffer, uint16_t bufferSize, Severity severity, int line, const char* fileName,
               uint16_t fileNameSize, const char* func, uint16_t funcNameSize, uint32_t timestamp_us, ISink& sink)
    : buffer_(buffer), buffer_size_(bufferSize), severity_(severity), sink_(sink) {
    auto width = std::max(0, (int)(MIN_HEADER_SIZE - 2) - 1 - TIME_S_WIDTH - 1 - TIME_US_WIDTH - 1 -
                                 1 - SEVERITY_WIDTH - 1 - 1 - fileNameSize - 2 - funcNameSize - 2);
    auto res = snprintf(buffer_, buffer_size_, "[%4d.%06d][%c][%s::%s::%-*d] ",
                        int(timestamp_us / 1000000), int(timestamp_us % 1000000),
                        severityChar(severity), fileName, func, width, line);
    position_ = std::min(buffer_size_ - 1, res);
    max_safe_position_ = buffer_size_ - MAX_TOKEN;
}

Record::~Record() {
    append('\n');
    sink_.write(buffer_, position_);
    if (severity_ == Severity::Fatal)
        sink_.flush();
    position_ = 0;
}

Record& Record::operator<<(const char* value) {
    if (position_ > max_safe_position_)
        return *this;

    size_t available = buffer_size_ - MAX_TOKEN - position_;
    size_t len = strnlen(value, available);
    memcpy(buffer_ + position_, value, len);
    position_ += static_cast<uint16_t>(len);
    return *this;
}

Record& Record::operator<<(bool value) {
    if (position_ > max_safe_position_)
        return *this;
    position_ += detail::toCString(buffer_ + position_, buffer_ + buffer_size_, value);
    return *this;
}

Record& Record::operator<<(float value) {
    if (position_ > max_safe_position_)
        return *this;
    position_ += detail::toCString(buffer_ + position_, buffer_ + buffer_size_, FloatArg{value, 2});
    return *this;
}

Record& Record::operator<<(HexArg arg) {
    if (position_ > max_safe_position_)
        return *this;
    buffer_[position_++] = 'x';
    position_ += detail::toCString(buffer_ + position_, buffer_ + buffer_size_, arg);
    return *this;
}

Record& Record::operator<<(BinArg arg) {
    if (position_ > max_safe_position_)
        return *this;
    buffer_[position_++] = 'b';
    position_ += detail::toCString(buffer_ + position_, buffer_ + buffer_size_, arg);
    return *this;
}

Record& Record::operator<<(FloatArg arg) {
    if (position_ > max_safe_position_)
        return *this;
    position_ += detail::toCString(buffer_ + position_, buffer_ + buffer_size_, arg);
    return *this;
}

}  // namespace ullog