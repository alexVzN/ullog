# ullog

[![Build & test](https://github.com/alexVzN/ullog/actions/workflows/ci.yml/badge.svg)](https://github.com/alexVzN/ullog/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
![C++](https://img.shields.io/badge/C%2B%2B-17%20%7C%2020%20%7C%2023-blue.svg)
![ARM Cortex-M](https://img.shields.io/badge/ARM%20Cortex--M-M0%2B%20%7C%20M4%20%7C%20M7%20%7C%20M33-blueviolet.svg)
![Dependencies](https://img.shields.io/badge/Dependencies-None-brightgreen.svg)

Tiny streaming logger for embedded C++ (C++17). Zero heap, zero exceptions,
no third-party dependencies.

## Features

- Header-driven RAII record: built in place, flushed in destructor.
- Stream API: `LOG << "x=" << x << " hex=" << ullog::Hex(addr, 8);`
- Built-in formatters: integers (signed/unsigned), booleans, floats,
  hex, binary, pointers.
- Compile-time bounded buffer, no allocations.
- Pluggable clock and sink via two small interfaces.

## Quick start

```cpp
#include <ullog/logger.h>

class UartSink : public ullog::ISink {
public:
    void write(const char* data, uint16_t len) override {
        HAL_UART_Transmit(&huart1, (uint8_t*)data, len, HAL_MAX_DELAY);
    }
};

class HalClock : public ullog::IClock {
public:
    uint32_t nowUs() override { return get_us_ticks(); }
};

static HalClock clock;
static UartSink sink;
static ullog::Logger<1024> logger(clock, sink);

void someFunc() {
    LOG << "armed=" << true << " v=" << ullog::Float(3.7f, 2);
}
```

## CMake

```cmake
add_subdirectory(third_party/ullog)
target_link_libraries(my_target PRIVATE ullog::ullog)
```

To build tests:

```bash
cmake -S . -B build -DULLOG_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## License

MIT — see [LICENSE](LICENSE).