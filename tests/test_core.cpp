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

static int mod3(const int& v) { return v % 3; }

TEST_CASE("MoveSemantics: move constructor from empty source yields empty destination", "[MoveSemantics]") {
    Queue src(identity);
    Queue dst(std::move(src));
    REQUIRE(dst.empty());
    REQUIRE(dst.size() == 0);
    REQUIRE_FALSE(dst.dequeue().has_value());
    REQUIRE(src.empty());
    REQUIRE(src.size() == 0);
}

TEST_CASE("MoveSemantics: move constructor transfers elements in correct order", "[MoveSemantics]") {
    Queue src(mod3);
    src.enqueue(1);  // key 1, group A
    src.enqueue(2);  // key 2, group B
    src.enqueue(4);  // key 1, joins group A
    Queue dst(std::move(src));
    REQUIRE(src.empty());
    REQUIRE(src.size() == 0);
    REQUIRE(dst.size() == 3);
    REQUIRE(*dst.dequeue() == 1);
    REQUIRE(*dst.dequeue() == 4);
    REQUIRE(*dst.dequeue() == 2);
    REQUIRE(dst.empty());
}

TEST_CASE("MoveSemantics: enqueue into destination after move uses transferred index", "[MoveSemantics]") {
    Queue src(mod3);
    src.enqueue(1);  // key 1, group A
    src.enqueue(2);  // key 2, group B
    Queue dst(std::move(src));
    dst.enqueue(4);  // key 1 — should join the transferred group A
    REQUIRE(dst.size() == 3);
    REQUIRE(*dst.dequeue() == 1);
    REQUIRE(*dst.dequeue() == 4);
    REQUIRE(*dst.dequeue() == 2);
}

TEST_CASE("MoveSemantics: move assignment self-assign is a no-op", "[MoveSemantics]") {
    Queue q(mod3);
    q.enqueue(10);
    q.enqueue(20);
    Queue& self = q;
    q = std::move(self);
    REQUIRE(q.size() == 2);
    REQUIRE(*q.dequeue() == 10);
    REQUIRE(*q.dequeue() == 20);
}

TEST_CASE("MoveSemantics: move assignment overwrites destination with source elements", "[MoveSemantics]") {
    Queue src(mod3);
    src.enqueue(1);
    src.enqueue(4);  // key 1, same group
    Queue dst(mod3);
    dst.enqueue(99);
    dst = std::move(src);
    REQUIRE(src.empty());
    REQUIRE(src.size() == 0);
    REQUIRE(dst.size() == 2);
    REQUIRE(*dst.dequeue() == 1);
    REQUIRE(*dst.dequeue() == 4);
    REQUIRE(dst.empty());
}
