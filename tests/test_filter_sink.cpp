// Runtime per-sink filtering test.
//
// Independent of the compile-time threshold (MAX_SEV): exercises ISink::submit
// directly, so it verifies the runtime gate built from the sink's accepted mask
// — the mechanism behind multi-sink routing. A record reaches write() only when
// its severity is a member of the sink's accepted mask.

#include <gtest/gtest.h>

#include <string>

#include "ullog/interfaces/i_sink.h"
#include "ullog/severity.h"

namespace {

using ullog::Severity;

class CaptureSink : public ullog::ISink {
public:
    using ullog::ISink::ISink;

    std::string out;
    void write(const char* msg, uint16_t len) override { out.append(msg, len); }
    void flush() override {}

    bool delivered(Severity severity) {
        out.clear();
        submit(severity, "x", 1);
        return !out.empty();
    }
};

constexpr Severity kAll[] = {Severity::Fatal, Severity::Error, Severity::Warn, Severity::Info, Severity::Debug};

}  // namespace

TEST(SinkFilterTest, DefaultMask_AcceptsEverySeverity) {
    CaptureSink sink;
    for (Severity s : kAll)
        EXPECT_TRUE(sink.delivered(s));
}

TEST(SinkFilterTest, ExplicitAll_AcceptsEverySeverity) {
    CaptureSink sink(Severity::All);
    for (Severity s : kAll)
        EXPECT_TRUE(sink.delivered(s));
}

TEST(SinkFilterTest, NoneMask_RejectsEverySeverity) {
    CaptureSink sink(Severity::None);
    for (Severity s : kAll)
        EXPECT_FALSE(sink.delivered(s));
}

TEST(SinkFilterTest, SingleSeverityMask_AcceptsOnlyThatSeverity) {
    CaptureSink sink(Severity::Error);
    EXPECT_TRUE(sink.delivered(Severity::Error));
    EXPECT_FALSE(sink.delivered(Severity::Fatal));
    EXPECT_FALSE(sink.delivered(Severity::Warn));
    EXPECT_FALSE(sink.delivered(Severity::Info));
    EXPECT_FALSE(sink.delivered(Severity::Debug));
}

TEST(SinkFilterTest, CombinedMask_AcceptsMembersRejectsRest) {
    CaptureSink sink(Severity::Error | Severity::Fatal);
    EXPECT_TRUE(sink.delivered(Severity::Fatal));
    EXPECT_TRUE(sink.delivered(Severity::Error));
    EXPECT_FALSE(sink.delivered(Severity::Warn));
    EXPECT_FALSE(sink.delivered(Severity::Info));
    EXPECT_FALSE(sink.delivered(Severity::Debug));
}

TEST(SinkFilterTest, ThresholdStyleMask_AcceptsSeverityAndAbove) {
    CaptureSink sink(Severity::Fatal | Severity::Error | Severity::Warn);
    EXPECT_TRUE(sink.delivered(Severity::Fatal));
    EXPECT_TRUE(sink.delivered(Severity::Error));
    EXPECT_TRUE(sink.delivered(Severity::Warn));
    EXPECT_FALSE(sink.delivered(Severity::Info));
    EXPECT_FALSE(sink.delivered(Severity::Debug));
}

TEST(SinkFilterTest, Submit_ForwardsPayloadUnchanged) {
    CaptureSink sink(Severity::Info);
    sink.submit(Severity::Info, "hello", 5);
    EXPECT_EQ(sink.out, "hello");
}
