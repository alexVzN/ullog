// MAX_SEV = None → every LOGx is gated at compile time, Fatal included.

#include <gtest/gtest.h>

#include <string>

#include "ullog/ullog.h"

namespace {

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

ULLOG_CREATE_DEFAULT(256, ullog::Severity::None, g_clock, g_sink);

static_assert(std::remove_reference_t<decltype(ULLOG_DEFAULT)>::max_severity == ullog::Severity::None);

class FilterNoneTest : public ::testing::Test {
protected:
    void SetUp() override {
        g_sink.out.clear();
        g_eval_count = 0;
    }
};

TEST_F(FilterNoneTest, Debug_Gated) {
    LOGD << tracked();
    EXPECT_TRUE(g_sink.out.empty());
    EXPECT_EQ(g_eval_count, 0);
}

TEST_F(FilterNoneTest, Info_Gated) {
    LOGI << tracked();
    EXPECT_TRUE(g_sink.out.empty());
    EXPECT_EQ(g_eval_count, 0);
}

TEST_F(FilterNoneTest, Warn_Gated) {
    LOGW << tracked();
    EXPECT_TRUE(g_sink.out.empty());
    EXPECT_EQ(g_eval_count, 0);
}

TEST_F(FilterNoneTest, Error_Gated) {
    LOGE << tracked();
    EXPECT_TRUE(g_sink.out.empty());
    EXPECT_EQ(g_eval_count, 0);
}

TEST_F(FilterNoneTest, Fatal_Gated) {
    LOGF << tracked();
    EXPECT_TRUE(g_sink.out.empty());
    EXPECT_EQ(g_eval_count, 0);
}
