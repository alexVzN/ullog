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
- Stream API: `LOGI << "x=" << x << " hex=" << ullog::Hex(addr, 8);`
- Five severity levels (`Debug → Fatal`) with single-char tag in the
  record header, filtered at compile time by a per-logger threshold —
  disabled `LOGx` sites do not evaluate their streamed arguments.
- Built-in formatters: integers (signed/unsigned), booleans, floats,
  hex, binary, pointers.
- Fan-out to several destinations with `MultiSink`, each child filtered
  by its own runtime severity mask.
- Compile-time bounded buffer, no allocations.
- Pluggable clock and sink via two small interfaces.

## Quick start

```cpp
#include <ullog/ullog.h>

class UartSink : public ullog::ISink {
public:
    void write(const char* data, uint16_t len) override {
        HAL_UART_Transmit(&huart1, (uint8_t*)data, len, HAL_MAX_DELAY);
    }
    void flush() override {}
};

class HalClock : public ullog::IClock {
public:
    uint32_t nowUs() override { return get_us_ticks(); }
};

static HalClock clock;
static UartSink sink;

// Buffer 1024 B, threshold = Warn → Debug/Info eliminated at compile time.
ULLOG_CREATE_DEFAULT(1024, ullog::Severity::Warn, clock, sink);

void someFunc() {
    LOGD << "skipped — eliminated, expensive_call() never runs";
    LOGW << "armed=" << true << " v=" << ullog::Float(3.7f, 2);
    LOGF << "boom";   // also flushes the sink
}
```

`ULLOG_CREATE_DEFAULT` is called once at namespace scope, typically in
the same header your project includes everywhere logging is needed. It
hides the logger instance behind `ULLOG_DEFAULT` — the `LOGx` macros pick
it up automatically, you never name the object.

## Severity levels

`Severity` is both a level and a bitmask. `Fatal` is the most severe
(lowest numeric value), `Debug` the least. `MAX_SEV` is the
*least-severe* level still emitted: every `LOGx` whose severity value is
strictly greater than `MAX_SEV` is gated out at compile time.

| Level    | Tag | Value  | Notes                                  |
|----------|:---:|:------:|----------------------------------------|
| `Fatal`  | `F` | `0x01` | Flushes the sink after writing         |
| `Error`  | `E` | `0x02` |                                        |
| `Warn`   | `W` | `0x04` |                                        |
| `Info`   | `I` | `0x08` |                                        |
| `Debug`  | `D` | `0x10` |                                        |
| `None`   |  —  | `0x00` | Sentinel: pass `None` to gate every LOGx, including Fatal |
| `All`    |  —  | `0xFF` | Sentinel: pass `All` to emit every LOGx (default)         |

## Per-sink filtering

Beyond the compile-time `MAX_SEV` threshold, each sink carries its own
*runtime* accepted mask — an arbitrary set of severities, passed to the
`ISink` constructor (default `All`). A sink opts in by inheriting the
constructor:

```cpp
class FlashSink : public ullog::ISink {
public:
    using ullog::ISink::ISink;   // FlashSink(Severity accepted)
    void write(const char* data, uint16_t len) override { /* ... */ }
    void flush() override {}
};
```

Unlike `MAX_SEV` (one ordered threshold per logger), the mask is a free set,
e.g. `Error | Fatal`. It is checked *after* formatting, so it is for routing,
not for cheap suppression — keep `MAX_SEV` as tight as the build allows,
since anything it gates out costs nothing at all.

## Multiple sinks

`MultiSink` drives several destinations from one logger. It is a pure router:
every record is forwarded to each child's `submit()`, and each child's own
mask decides whether it is written.

```cpp
RttSink   rtt;                                                   // accepts All
FlashSink flash(ullog::Severity::Error | ullog::Severity::Fatal);

ullog::MultiSink sink(rtt, flash);   // child count deduced from arguments
ULLOG_CREATE_DEFAULT(1024, ullog::Severity::All, clock, sink);

LOGI << "telemetry";   // → rtt only
LOGE << "fault";       // → rtt and flash
```

Children are held by reference — they must outlive the `MultiSink`.

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
