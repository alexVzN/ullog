#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "ullog/logger.h"

namespace {

class CaptureSink : public ullog::ISink {
public:
    std::string out;
    void write(const char* msg, uint16_t len) override { out.append(msg, len); }
    void flush() override {}
};

class FakeClock : public ullog::IClock {
public:
    uint32_t ticks_us = 0;
    uint32_t nowUs() override { return ticks_us; }
};

}  // namespace

class LoggerTest : public ::testing::Test {
protected:
    static constexpr size_t kBufSize = 256;

    void SetUp() override {
        sink_.out.clear();
        clock_.ticks_us = 0;
        logger_ = std::make_unique<ullog::Logger<kBufSize>>(clock_, sink_);
    }

    void TearDown() override { logger_.reset(); }

    bool outputContains(const std::string& s) const {
        return sink_.out.find(s) != std::string::npos;
    }

    CaptureSink sink_;
    FakeClock clock_;
    std::unique_ptr<ullog::Logger<kBufSize>> logger_;
};

// ── ILogger instance ──────────────────────────────────────────────────────────

TEST_F(LoggerTest, Get_ReturnsConstructedInstance) {
    EXPECT_EQ(&ullog::ILogger::get(), logger_.get());
}

// ── Sink timing ───────────────────────────────────────────────────────────────

TEST_F(LoggerTest, Sink_NotCalledBeforeRecordDestruction) {
    {
        auto rec = LOG;  // Record alive
        EXPECT_TRUE(sink_.out.empty());
        // rec destroyed here → sink fires
    }
    EXPECT_FALSE(sink_.out.empty());
}

TEST_F(LoggerTest, Sink_CalledOncePerRecord) {
    LOG << "hello";
    int count = 0;
    for (char c : sink_.out)
        if (c == '\n')
            ++count;

    EXPECT_EQ(count, 1);
}

TEST_F(LoggerTest, Sink_CalledForEachRecord) {
    LOG << "first";
    auto after_first = sink_.out.size();
    LOG << "second";
    EXPECT_GT(sink_.out.size(), after_first);
}

// ── Record format ─────────────────────────────────────────────────────────────

TEST_F(LoggerTest, Record_EndsWithNewline) {
    { auto r = LOG; }
    ASSERT_FALSE(sink_.out.empty());
    EXPECT_EQ(sink_.out.back(), '\n');
}

TEST_F(LoggerTest, Record_Header_ContainsBrackets) {
    { auto r = LOG; }
    EXPECT_EQ(sink_.out.front(), '[');
}

TEST_F(LoggerTest, Record_Header_TimestampZeroSeconds) {
    clock_.ticks_us = 0;
    { auto r = LOG; }
    EXPECT_TRUE(outputContains("0.000000"));
}

TEST_F(LoggerTest, Record_Header_TimestampOneSecond) {
    clock_.ticks_us = 1000000;
    { auto r = LOG; }
    EXPECT_TRUE(outputContains("1.000000"));
}

TEST_F(LoggerTest, Record_Header_TimestampSubSecond) {
    clock_.ticks_us = 500000;
    { auto r = LOG; }
    EXPECT_TRUE(outputContains("0.500000"));
}

// ── operator<< types ──────────────────────────────────────────────────────────

TEST_F(LoggerTest, Stream_CString) {
    LOG << "hello world";
    EXPECT_TRUE(outputContains("hello world"));
}

TEST_F(LoggerTest, Stream_StringLiteral) {
    LOG << "literal";
    EXPECT_TRUE(outputContains("literal"));
}

TEST_F(LoggerTest, Stream_BoolTrue) {
    LOG << true;
    EXPECT_TRUE(outputContains("true"));
}

TEST_F(LoggerTest, Stream_BoolFalse) {
    LOG << false;
    EXPECT_TRUE(outputContains("false"));
}

TEST_F(LoggerTest, Stream_PositiveInt) {
    LOG << static_cast<int32_t>(42);
    EXPECT_TRUE(outputContains("42"));
}

TEST_F(LoggerTest, Stream_NegativeInt) {
    LOG << static_cast<int32_t>(-7);
    EXPECT_TRUE(outputContains("-7"));
}

TEST_F(LoggerTest, Stream_Uint) {
    LOG << static_cast<uint32_t>(255);
    EXPECT_TRUE(outputContains("255"));
}

TEST_F(LoggerTest, Stream_Float) {
    LOG << 3.14f;
    EXPECT_TRUE(outputContains("3.14"));
}

TEST_F(LoggerTest, Stream_HexArg) {
    LOG << ullog::Hex(0xFF, 2);
    EXPECT_TRUE(outputContains("FF") || outputContains("ff"));
}

TEST_F(LoggerTest, Stream_BinArg) {
    LOG << ullog::Bin(0b1010, 4);
    EXPECT_TRUE(outputContains("1010"));
}

TEST_F(LoggerTest, Stream_FloatArg) {
    LOG << ullog::Float(1.5f, 1);
    EXPECT_TRUE(outputContains("1.5"));
}

TEST_F(LoggerTest, Stream_ChainedValues) {
    LOG << "x=" << static_cast<int32_t>(10) << " ok";
    EXPECT_TRUE(outputContains("x="));
    EXPECT_TRUE(outputContains("10"));
    EXPECT_TRUE(outputContains("ok"));
}

// ── Buffer overflow protection ────────────────────────────────────────────────

TEST_F(LoggerTest, BufferOverflow_DoesNotCrash) {
    std::string big(512, 'A');
    EXPECT_NO_FATAL_FAILURE({ LOG << big.c_str(); });
}

TEST_F(LoggerTest, BufferOverflow_OutputLengthBounded) {
    std::string big(512, 'B');
    LOG << big.c_str();
    EXPECT_LE(sink_.out.size(), kBufSize + ullog::MAX_TOKEN);
}