// Compile-time filtering test.
//
// Verifies that with the logger initialised via
//   ULLOG_CREATE_DEFAULT(SIZE, Severity::Error, clock, sink);
// the following holds:
//   * disabled severities (Debug/Info/Warn) produce no output AND never
//     evaluate their streamed arguments — the ternary's dead branch is
//     dropped by the optimizer;
//   * the enabled severity (Error) produces output and evaluates arguments;
//   * Fatal is unconditionally compiled in regardless of threshold.
//
// The threshold is set per-call-to-ULLOG_CREATE_DEFAULT — no #define, no
// project-wide knob, and the user never types the logger's name.

#include <gtest/gtest.h>

#include <string>

#include "ullog/ullog.h"

namespace {

// Incremented only when its return value is actually evaluated.
int g_eval_count = 0;
const char* tracked() {
    ++g_eval_count;
    return "tracked";
}

class CaptureSink : public ullog::ISink {
public:
    std::string out;
    void write(const char* msg, uint16_t len) override { out.append(msg, len); }
    void flush() override {}
};

class FakeClock : public ullog::IClock {
public:
    uint32_t nowUs() override { return 0; }
};

CaptureSink g_sink;
FakeClock g_clock;

}  // namespace

// Threshold = Error → Debug/Info/Warn fall below it and must be eliminated.
ULLOG_CREATE_DEFAULT(256, ullog::Severity::Error, g_clock, g_sink);

class FilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        g_sink.out.clear();
        g_eval_count = 0;
    }

    bool outputContains(const std::string& s) const { return g_sink.out.find(s) != std::string::npos; }
};

// ── Disabled severities: no output, no argument evaluation ──────────────────

TEST_F(FilterTest, Debug_DisabledNoOutputNoEval) {
    LOGD << tracked();
    EXPECT_TRUE(g_sink.out.empty());
    EXPECT_EQ(g_eval_count, 0);
}

TEST_F(FilterTest, Info_DisabledNoOutputNoEval) {
    LOGI << tracked();
    EXPECT_TRUE(g_sink.out.empty());
    EXPECT_EQ(g_eval_count, 0);
}

TEST_F(FilterTest, Warn_DisabledNoOutputNoEval) {
    LOGW << tracked();
    EXPECT_TRUE(g_sink.out.empty());
    EXPECT_EQ(g_eval_count, 0);
}

// ── Enabled severity: output present, argument evaluated ────────────────────

TEST_F(FilterTest, Error_EnabledOutputAndEval) {
    LOGE << tracked();
    EXPECT_TRUE(outputContains("tracked"));
    EXPECT_TRUE(outputContains("][E]["));
    EXPECT_EQ(g_eval_count, 1);
}

// ── Fatal is always compiled in regardless of threshold ─────────────────────

TEST_F(FilterTest, Fatal_AlwaysCompiledIn) {
    LOGF << tracked();
    EXPECT_TRUE(outputContains("tracked"));
    EXPECT_TRUE(outputContains("][F]["));
    EXPECT_EQ(g_eval_count, 1);
}

// ── max_severity reflects the ULLOG_CREATE_DEFAULT argument ─────────────────

TEST_F(FilterTest, Logger_MaxSeverityReflectsInitArg) {
    static_assert(std::remove_reference_t<decltype(ULLOG_DEFAULT)>::max_severity == ullog::Severity::Error);
    SUCCEED();
}
