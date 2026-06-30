// ALL_EXCEPT(...) builds an accepted mask of every severity minus the listed ones.
//
// Pure mask arithmetic plus its intended use: feeding the result into a sink's
// accepted mask so the listed severities are filtered out at runtime.

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

}  // namespace

// Contract: the listed severities are cleared from the All mask, the rest stay set.
// Asserted against the mask bits (not an exact literal) so adding a severity needs no edit here.
static_assert(ALL_EXCEPT() == Severity::All);
static_assert((ALL_EXCEPT(Severity::Debug) & Severity::Debug) == Severity::None);
static_assert((ALL_EXCEPT(Severity::Debug) & Severity::Info) == Severity::Info);
static_assert((ALL_EXCEPT(Severity::Debug, Severity::Info) & (Severity::Debug | Severity::Info)) == Severity::None);

constexpr Severity kAll[] = {Severity::Fatal, Severity::Error, Severity::Warn, Severity::Info, Severity::Debug};

TEST(AllExceptTest, NoArgs_DeliversEverySeverity) {
    CaptureSink sink(ALL_EXCEPT());
    for (Severity s : kAll)
        EXPECT_TRUE(sink.delivered(s));
}

TEST(AllExceptTest, ExcludingEverySeverity_DeliversNothing) {
    CaptureSink sink(ALL_EXCEPT(Severity::Fatal, Severity::Error, Severity::Warn, Severity::Info, Severity::Debug));
    for (Severity s : kAll)
        EXPECT_FALSE(sink.delivered(s));
}

TEST(AllExceptTest, ExcludesSingleSeverity) {
    CaptureSink sink(ALL_EXCEPT(Severity::Debug));
    EXPECT_TRUE(sink.delivered(Severity::Fatal));
    EXPECT_TRUE(sink.delivered(Severity::Error));
    EXPECT_TRUE(sink.delivered(Severity::Warn));
    EXPECT_TRUE(sink.delivered(Severity::Info));
    EXPECT_FALSE(sink.delivered(Severity::Debug));
}

TEST(AllExceptTest, ExcludesMultipleSeverities) {
    CaptureSink sink(ALL_EXCEPT(Severity::Debug, Severity::Info));
    EXPECT_TRUE(sink.delivered(Severity::Fatal));
    EXPECT_TRUE(sink.delivered(Severity::Error));
    EXPECT_TRUE(sink.delivered(Severity::Warn));
    EXPECT_FALSE(sink.delivered(Severity::Info));
    EXPECT_FALSE(sink.delivered(Severity::Debug));
}

TEST(AllExceptTest, OrderIndependent) {
    EXPECT_EQ(ALL_EXCEPT(Severity::Warn, Severity::Fatal), ALL_EXCEPT(Severity::Fatal, Severity::Warn));
}

TEST(AllExceptTest, DuplicateArgsIdempotent) {
    EXPECT_EQ(ALL_EXCEPT(Severity::Warn, Severity::Warn), ALL_EXCEPT(Severity::Warn));
}
