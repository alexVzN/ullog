#pragma once

#include <cstdint>

namespace ullog {

// Max bytes any single operator<< can write (BinArg: 'b' + 32 bits = 33).
inline constexpr uint16_t MAX_TOKEN = 40;

struct HexArg {
    uint32_t value;
    int width;
};
struct BinArg {
    uint32_t value;
    int width;
};
struct FloatArg {
    float value;
    int precision;
};

inline HexArg Hex(uint32_t v, int width = 0) {
    return {v, width};
}
inline BinArg Bin(uint32_t v, int width = 0) {
    return {v, width};
}
inline FloatArg Float(float v, int precision) {
    return {v, precision};
}

constexpr const char* extractFileNameOnly(const char* str) {
    auto it = str;
    auto lastSlash = str;

    while (*it) {
        if (*it == '/' || *it == '\\')
            lastSlash = it + 1;
        ++it;
    }

    return lastSlash;
}

namespace detail {

uint8_t toCString(char* begin, char* end, bool value);
uint8_t toCString(char* begin, char* end, int32_t value);
uint8_t toCString(char* begin, char* end, uint32_t value);
uint8_t toCString(char* begin, char* end, HexArg arg);
uint8_t toCString(char* begin, char* end, BinArg arg);
uint8_t toCString(char* begin, char* end, FloatArg arg);

}  // namespace detail
}  // namespace ullog