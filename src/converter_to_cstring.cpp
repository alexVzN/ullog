#include "ullog/converter_to_cstring.h"

#include <cstring>

namespace ullog::detail {

uint8_t toCString(char* begin, char* end, bool value) {
    if (begin >= end)
        return 0;

    const char* str = value ? "true" : "false";
    uint8_t len = value ? 4 : 5;
    uint8_t available = static_cast<uint8_t>(end - begin);
    uint8_t toCopy = len < available ? len : available;
    memcpy(begin, str, toCopy);
    return toCopy;
}

uint8_t toCString(char* begin, char* end, int32_t value) {
    if (begin >= end)
        return 0;

    char* p = begin;

    uint32_t uval;
    if (value < 0) {
        *p++ = '-';
        if (p >= end)
            return 1;

        uval = static_cast<uint32_t>(-(value + 1)) + 1u;
    } else {
        uval = static_cast<uint32_t>(value);
    }

    p += toCString(p, end, uval);
    return static_cast<uint8_t>(p - begin);
}

uint8_t toCString(char* begin, char* end, uint32_t value) {
    if (begin >= end)
        return 0;

    char* p = begin;

    char tmp[10];
    uint8_t len = 0;
    if (value == 0) {
        tmp[len++] = '0';
    } else {
        while (value > 0) {
            tmp[len++] = '0' + (value % 10);
            value /= 10;
        }
    }

    for (int8_t i = len - 1; i >= 0 && p < end; --i)
        *p++ = tmp[i];

    return static_cast<uint8_t>(p - begin);
}

uint8_t toCString(char* begin, char* end, HexArg arg) {
    if (begin >= end)
        return 0;
    static constexpr char hex[] = "0123456789ABCDEF";
    char* p = begin;

    char tmp[8];
    uint8_t len = 0;
    uint32_t value = arg.value;
    if (value == 0) {
        tmp[len++] = '0';
    } else {
        while (value > 0) {
            tmp[len++] = hex[value & 0xF];
            value >>= 4;
        }
    }

    int8_t pad = static_cast<int8_t>(arg.width) - static_cast<int8_t>(len);
    while (pad-- > 0 && p < end)
        *p++ = '0';

    for (int8_t i = len - 1; i >= 0 && p < end; --i)
        *p++ = tmp[i];

    return static_cast<uint8_t>(p - begin);
}

uint8_t toCString(char* begin, char* end, BinArg arg) {
    if (begin >= end)
        return 0;
    char* p = begin;

    int bits;
    if (arg.width > 0) {
        bits = arg.width;
    } else if (arg.value == 0) {
        bits = 1;
    } else {
#if defined(__GNUC__) || defined(__clang__)
        bits = 32 - __builtin_clz(arg.value);
#else
        bits = 1;
        for (int8_t i = 31; i >= 1; --i) {
            if ((arg.value >> i) & 1) {
                bits = i + 1;
                break;
            }
        }
#endif
    }

    for (int i = bits - 1; i >= 0 && p < end; --i)
        *p++ = (arg.value >> i) & 1 ? '1' : '0';

    return static_cast<uint8_t>(p - begin);
}

uint8_t toCString(char* begin, char* end, FloatArg arg) {
    if (begin >= end)
        return 0;
    char* p = begin;

    float value = arg.value;

    if (value < 0.0f) {
        *p++ = '-';
        if (p >= end)
            return 1;
        value = -value;
    }

    float rounding = 0.5f;
    for (int i = 0; i < arg.precision; ++i)
        rounding /= 10.0f;

    value += rounding;

    uint32_t intPart = static_cast<uint32_t>(value);
    p += toCString(p, end, intPart);

    if (arg.precision <= 0 || p >= end)
        return static_cast<uint8_t>(p - begin);

    *p++ = '.';

    float frac = value - static_cast<float>(intPart);
    for (int i = 0; i < arg.precision && p < end; ++i) {
        frac *= 10.0f;
        uint8_t digit = static_cast<uint8_t>(frac);
        *p++ = '0' + digit;
        frac -= digit;
    }

    return static_cast<uint8_t>(p - begin);
}

}  // namespace ullog::detail