#include <gtest/gtest.h>

#include <string>

#include "ullog/ullog.h"

namespace {

class CaptureSink : public ullog::ISink {
public:
    std::string out;
    int flush_count = 0;
    void write(const char* msg, uint16_t len) override { out.append(msg, len); }
    void flush() override { ++flush_count; }
};

class FakeClock : public ullog::IClock {
public:
    uint32_t ticks_us = 0;
    uint32_t nowUs() override { return ticks_us; }
};

CaptureSink g_sink;
FakeClock g_clock;

}  // namespace

ULLOG_CREATE_DEFAULT(256, ullog::Severity::All, g_clock, g_sink);

static_assert(::ullog::detail::kMaxSeverity == ullog::Severity::All);

class LoggerTest : public ::testing::Test {
protected:
    static constexpr size_t kBufSize = 256;

    void SetUp() override {
        g_sink.out.clear();
        g_sink.flush_count = 0;
        g_clock.ticks_us = 0;
    }

    bool outputContains(const std::string& s) const { return g_sink.out.find(s) != std::string::npos; }
};

TEST_F(LoggerTest, Sink_FiresOnStatementEnd) {
    LOGI;
    EXPECT_FALSE(g_sink.out.empty());
}

TEST_F(LoggerTest, Sink_CalledOncePerRecord) {
    LOGI << "hello";
    int count = 0;
    for (char c : g_sink.out)
        if (c == '\n')
            ++count;
    EXPECT_EQ(count, 1);
}

TEST_F(LoggerTest, Sink_CalledForEachRecord) {
    LOGI << "first";
    auto after_first = g_sink.out.size();
    LOGI << "second";
    EXPECT_GT(g_sink.out.size(), after_first);
}

TEST_F(LoggerTest, Record_EndsWithNewline) {
    LOGI;
    ASSERT_FALSE(g_sink.out.empty());
    EXPECT_EQ(g_sink.out.back(), '\n');
}

TEST_F(LoggerTest, Record_Header_ContainsBrackets) {
    LOGI;
    EXPECT_EQ(g_sink.out.front(), '[');
}

TEST_F(LoggerTest, Record_Header_TimestampZeroSeconds) {
    g_clock.ticks_us = 0;
    LOGI;
    EXPECT_TRUE(outputContains("0.000000"));
}

TEST_F(LoggerTest, Record_Header_TimestampOneSecond) {
    g_clock.ticks_us = 1000000;
    LOGI;
    EXPECT_TRUE(outputContains("1.000000"));
}

TEST_F(LoggerTest, Record_Header_TimestampSubSecond) {
    g_clock.ticks_us = 500000;
    LOGI;
    EXPECT_TRUE(outputContains("0.500000"));
}

TEST_F(LoggerTest, Stream_CString) {
    LOGI << "hello world";
    EXPECT_TRUE(outputContains("hello world"));
}

TEST_F(LoggerTest, Stream_StringLiteral) {
    LOGI << "literal";
    EXPECT_TRUE(outputContains("literal"));
}

TEST_F(LoggerTest, Stream_BoolTrue) {
    LOGI << true;
    EXPECT_TRUE(outputContains("true"));
}

TEST_F(LoggerTest, Stream_BoolFalse) {
    LOGI << false;
    EXPECT_TRUE(outputContains("false"));
}

TEST_F(LoggerTest, Stream_PositiveInt) {
    LOGI << static_cast<int32_t>(42);
    EXPECT_TRUE(outputContains("42"));
}

TEST_F(LoggerTest, Stream_NegativeInt) {
    LOGI << static_cast<int32_t>(-7);
    EXPECT_TRUE(outputContains("-7"));
}

TEST_F(LoggerTest, Stream_Uint) {
    LOGI << static_cast<uint32_t>(255);
    EXPECT_TRUE(outputContains("255"));
}

TEST_F(LoggerTest, Stream_Float) {
    LOGI << 3.14f;
    EXPECT_TRUE(outputContains("3.14"));
}

TEST_F(LoggerTest, Stream_HexArg) {
    LOGI << ullog::Hex(0xFF, 2);
    EXPECT_TRUE(outputContains("FF") || outputContains("ff"));
}

TEST_F(LoggerTest, Stream_BinArg) {
    LOGI << ullog::Bin(0b1010, 4);
    EXPECT_TRUE(outputContains("1010"));
}

TEST_F(LoggerTest, Stream_FloatArg) {
    LOGI << ullog::Float(1.5f, 1);
    EXPECT_TRUE(outputContains("1.5"));
}

TEST_F(LoggerTest, Stream_ChainedValues) {
    LOGI << "x=" << static_cast<int32_t>(10) << " ok";
    EXPECT_TRUE(outputContains("x="));
    EXPECT_TRUE(outputContains("10"));
    EXPECT_TRUE(outputContains("ok"));
}

TEST_F(LoggerTest, BufferOverflow_DoesNotCrash) {
    std::string big(512, 'A');
    EXPECT_NO_FATAL_FAILURE({ LOGI << big.c_str(); });
}

TEST_F(LoggerTest, BufferOverflow_OutputLengthBounded) {
    std::string big(512, 'B');
    LOGI << big.c_str();
    EXPECT_LE(g_sink.out.size(), kBufSize + ullog::MAX_TOKEN);
}

TEST_F(LoggerTest, Severity_DebugTag) {
    LOGD << "x";
    EXPECT_TRUE(outputContains("][D]["));
}

TEST_F(LoggerTest, Severity_InfoTag) {
    LOGI << "x";
    EXPECT_TRUE(outputContains("][I]["));
}

TEST_F(LoggerTest, Severity_WarnTag) {
    LOGW << "x";
    EXPECT_TRUE(outputContains("][W]["));
}

TEST_F(LoggerTest, Severity_ErrorTag) {
    LOGE << "x";
    EXPECT_TRUE(outputContains("][E]["));
}

TEST_F(LoggerTest, Severity_FatalTag) {
    LOGF << "x";
    EXPECT_TRUE(outputContains("][F]["));
}

TEST_F(LoggerTest, Severity_CharHelper) {
    EXPECT_EQ(ullog::severityChar(ullog::Severity::Debug), 'D');
    EXPECT_EQ(ullog::severityChar(ullog::Severity::Info), 'I');
    EXPECT_EQ(ullog::severityChar(ullog::Severity::Warn), 'W');
    EXPECT_EQ(ullog::severityChar(ullog::Severity::Error), 'E');
    EXPECT_EQ(ullog::severityChar(ullog::Severity::Fatal), 'F');
}

TEST_F(LoggerTest, Fatal_FlushesSink) {
    LOGF << "boom";
    EXPECT_EQ(g_sink.flush_count, 1);
}

TEST_F(LoggerTest, NonFatal_DoesNotFlush) {
    LOGD << "d";
    LOGI << "i";
    LOGW << "w";
    LOGE << "e";
    EXPECT_EQ(g_sink.flush_count, 0);
}
