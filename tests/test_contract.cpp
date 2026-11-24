#include <catch2/catch_test_macros.hpp>
#include <israeli_queue/israeli_queue.hpp>
#include <memory>
#include <stdexcept>

// --- ExceptionSafety ---

struct ThrowOnN {
    int n;
    int count = 0;
    int operator()(const int& v) {
        if (++count >= n) throw std::runtime_error("extractor failed");
        return v % 3;
    }
};

TEST_CASE("ExceptionSafety: extractor throws leaves queue unchanged", "[ExceptionSafety]") {
    ThrowOnN extractor{3}; // throws on the 3rd call
    iq::IsraeliQueue<int, ThrowOnN> q(extractor);
    q.enqueue(1);
    q.enqueue(2);
    REQUIRE(q.size() == 2);

    REQUIRE_THROWS_AS(q.enqueue(3), std::runtime_error);

    // Queue must be unchanged
    REQUIRE(q.size() == 2);
    REQUIRE(*q.dequeue() == 1);
    REQUIRE(*q.dequeue() == 2);
    REQUIRE(q.empty());
}

// --- TypeRequirements ---

struct MoveOnly {
    int value;
    explicit MoveOnly(int v) : value(v) {}
    MoveOnly(MoveOnly&&)            = default;
    MoveOnly& operator=(MoveOnly&&) = default;
    MoveOnly(const MoveOnly&)            = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
};

static int move_only_key(const MoveOnly& m) { return m.value % 3; }

TEST_CASE("TypeRequirements: move-only type enqueues and dequeues", "[TypeRequirements]") {
    iq::IsraeliQueue<MoveOnly, int(*)(const MoveOnly&)> q(move_only_key);
    q.enqueue(MoveOnly{1});
    q.enqueue(MoveOnly{4}); // same group as 1
    auto a = q.dequeue();
    REQUIRE(a.has_value());
    REQUIRE(a->value == 1);
    auto b = q.dequeue();
    REQUIRE(b.has_value());
    REQUIRE(b->value == 4);
    REQUIRE(q.empty());
}

TEST_CASE("TypeRequirements: enqueue moves value into node", "[TypeRequirements]") {
    iq::IsraeliQueue<std::unique_ptr<int>, int(*)(const std::unique_ptr<int>&)> q(
        [](const std::unique_ptr<int>& p) { return *p % 3; }
    );
    auto ptr = std::make_unique<int>(42);
    q.enqueue(std::move(ptr));
    REQUIRE_FALSE(ptr); // ownership transferred
    auto result = q.dequeue();
    REQUIRE(result.has_value());
    REQUIRE(**result == 42);
}
