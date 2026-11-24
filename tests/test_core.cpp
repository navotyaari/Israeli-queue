#include <catch2/catch_test_macros.hpp>
#include <israeli_queue/israeli_queue.hpp>

using Queue = iq::IsraeliQueue<int, int(*)(const int&)>;

static int identity(const int& v) { return v; }

TEST_CASE("EmptyQueue: empty returns true on construction", "[EmptyQueue]") {
    Queue q(identity);
    REQUIRE(q.empty());
}

TEST_CASE("EmptyQueue: size is zero on construction", "[EmptyQueue]") {
    Queue q(identity);
    REQUIRE(q.size() == 0);
}

TEST_CASE("EmptyQueue: dequeue on empty returns nullopt", "[EmptyQueue]") {
    Queue q(identity);
    REQUIRE_FALSE(q.dequeue().has_value());
}

TEST_CASE("SingleElement: empty false after enqueue", "[SingleElement]") {
    Queue q(identity);
    q.enqueue(42);
    REQUIRE_FALSE(q.empty());
}

TEST_CASE("SingleElement: size one after enqueue", "[SingleElement]") {
    Queue q(identity);
    q.enqueue(42);
    REQUIRE(q.size() == 1);
}

TEST_CASE("SingleElement: front returns head value", "[SingleElement]") {
    Queue q(identity);
    q.enqueue(42);
    REQUIRE(q.front() == 42);
}

TEST_CASE("SingleElement: front does not remove element", "[SingleElement]") {
    Queue q(identity);
    q.enqueue(42);
    q.front();
    REQUIRE(q.size() == 1);
}

TEST_CASE("SingleElement: enqueue then dequeue round-trips value", "[SingleElement]") {
    Queue q(identity);
    q.enqueue(42);
    auto val = q.dequeue();
    REQUIRE(val.has_value());
    REQUIRE(*val == 42);
}

TEST_CASE("SingleElement: empty true after dequeue", "[SingleElement]") {
    Queue q(identity);
    q.enqueue(42);
    q.dequeue();
    REQUIRE(q.empty());
}

TEST_CASE("SingleElement: pop removes head", "[SingleElement]") {
    Queue q(identity);
    q.enqueue(42);
    q.pop();
    REQUIRE(q.empty());
}
