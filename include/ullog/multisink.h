#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <type_traits>

#include "interfaces/i_sink.h"
#include "severity.h"

namespace ullog {

template <std::size_t N>
class MultiSink : public ISink {
public:
    template <typename... Sinks>
    explicit MultiSink(Sinks&... sinks) : ISink(Severity::All), sinks_{sinks...} {
        static_assert((std::is_base_of_v<ISink, Sinks> && ...), "every argument must derive from ISink");
    }

    void submit(Severity severity, const char* data, uint16_t len) override {
        for (ISink& sink : sinks_)
            sink.submit(severity, data, len);
    }

    void flush() override {
        for (ISink& sink : sinks_)
            sink.flush();
    }

private:
    void write(const char* data, uint16_t len) override {}
    std::array<std::reference_wrapper<ISink>, N> sinks_;
};

template <typename... Sinks>
MultiSink(Sinks&...) -> MultiSink<sizeof...(Sinks)>;

}  // namespace ullog
