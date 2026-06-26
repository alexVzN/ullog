// MultiSink fan-out test.
//
// MultiSink is a pure router: it overrides submit() and forwards each record to
// every child's submit(), so each child applies its OWN accepted mask. MultiSink
// keeps no severity logic of its own — routing is decided entirely by the
// children. These tests pin that contract: broadcast, independent per-child
// filtering, flush fan-out, payload integrity, and dispatch through a base ref.

#include <gtest/gtest.h>

#include <string>
#include <type_traits>

#include "ullog/interfaces/i_sink.h"
#include "ullog/multisink.h"
#include "ullog/severity.h"

namespace {

using ullog::MultiSink;
using ullog::Severity;

class CaptureSink : public ullog::ISink {
public:
    using ullog::ISink::ISink;

    std::string out;
    int flushes = 0;

    void write(const char* msg, uint16_t len) override { out.append(msg, len); }
    void flush() override { ++flushes; }
};

}  // namespace

TEST(MultiSinkTest, Broadcast_ForwardsToEveryChild) {
    CaptureSink a, b, c;
    MultiSink sink(a, b, c);

    sink.submit(Severity::Info, "hello", 5);

    EXPECT_EQ(a.out, "hello");
    EXPECT_EQ(b.out, "hello");
    EXPECT_EQ(c.out, "hello");
}

TEST(MultiSinkTest, PerChildMask_RoutesIndependently) {
    CaptureSink errs(Severity::Error);
    CaptureSink infos(Severity::Info);
    MultiSink sink(errs, infos);

    sink.submit(Severity::Error, "e", 1);
    sink.submit(Severity::Info, "i", 1);

    EXPECT_EQ(errs.out, "e");
    EXPECT_EQ(infos.out, "i");
}

TEST(MultiSinkTest, ChildMaskRejects_DropsOnlyThatChild) {
    CaptureSink accepting(Severity::All);
    CaptureSink rejecting(Severity::None);
    MultiSink sink(accepting, rejecting);

    sink.submit(Severity::Warn, "w", 1);

    EXPECT_EQ(accepting.out, "w");
    EXPECT_TRUE(rejecting.out.empty());
}

TEST(MultiSinkTest, Flush_ForwardsToEveryChild) {
    CaptureSink a, b;
    MultiSink sink(a, b);

    sink.flush();

    EXPECT_EQ(a.flushes, 1);
    EXPECT_EQ(b.flushes, 1);
}

TEST(MultiSinkTest, SingleChild_Forwards) {
    CaptureSink a;
    MultiSink sink(a);

    sink.submit(Severity::Debug, "d", 1);

    EXPECT_EQ(a.out, "d");
}

TEST(MultiSinkTest, Payload_HonorsLengthNotTermination) {
    CaptureSink a;
    MultiSink sink(a);

    sink.submit(Severity::Info, "abcXYZ", 3);

    EXPECT_EQ(a.out, "abc");
}

TEST(MultiSinkTest, DeductionGuide_DeducesChildCount) {
    CaptureSink a, b;
    MultiSink sink(a, b);

    static_assert(std::is_same_v<decltype(sink), MultiSink<2>>);
    SUCCEED();
}

TEST(MultiSinkTest, DispatchesThroughBaseReference) {
    CaptureSink a, b;
    MultiSink sink(a, b);

    ullog::ISink& base = sink;
    base.submit(Severity::Error, "x", 1);

    EXPECT_EQ(a.out, "x");
    EXPECT_EQ(b.out, "x");
}
