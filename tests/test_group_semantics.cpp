#include <catch2/catch_test_macros.hpp>
#include <israeli_queue/israeli_queue.hpp>

using Queue = iq::IsraeliQueue<int, int(*)(const int&)>;

static int identity(const int& v) { return v; }
static int mod3(const int& v) { return v % 3; }

TEST_CASE("GroupFormation: two same-key elements dequeue consecutively", "[GroupFormation]") {
    Queue q(mod3);
    q.enqueue(3);  // key 0
    q.enqueue(1);  // key 1
    q.enqueue(6);  // key 0 — joins group of 3
    auto a = q.dequeue();
    auto b = q.dequeue();
    auto c = q.dequeue();
    REQUIRE(a.has_value());
    REQUIRE(b.has_value());
    REQUIRE(c.has_value());
    REQUIRE(*a == 3);
    REQUIRE(*b == 6);  // 6 joins group of 3, dequeued before 1
    REQUIRE(*c == 1);
}

TEST_CASE("GroupFormation: newcomer joins existing group not queue tail", "[GroupFormation]") {
    Queue q(mod3);
    q.enqueue(1);  // key 1, group A
    q.enqueue(2);  // key 2, group B
    q.enqueue(4);  // key 1 — joins group A, NOT after group B
    // Expected dequeue: 1, 4, 2
    REQUIRE(*q.dequeue() == 1);
    REQUIRE(*q.dequeue() == 4);
    REQUIRE(*q.dequeue() == 2);
}

TEST_CASE("GroupFormation: all same-key elements form single group", "[GroupFormation]") {
    Queue q(mod3);
    q.enqueue(3);
    q.enqueue(6);
    q.enqueue(9);
    REQUIRE(q.size() == 3);
    REQUIRE(*q.dequeue() == 3);
    REQUIRE(*q.dequeue() == 6);
    REQUIRE(*q.dequeue() == 9);
    REQUIRE(q.empty());
}

TEST_CASE("GroupFormation: newcomer rejoins after group drains and refills", "[GroupFormation]") {
    Queue q(mod3);
    q.enqueue(3);  // key 0
    q.enqueue(1);  // key 1
    q.dequeue();   // dequeues 3, group 0 drains
    q.enqueue(6);  // key 0 — group 0 drained; starts new group at back
    // Queue now: [1 (key1)] [6 (key0)]
    REQUIRE(*q.dequeue() == 1);
    REQUIRE(*q.dequeue() == 6);
}

// GroupOrdering tests

TEST_CASE("GroupOrdering: all unique keys dequeue in enqueue order", "[GroupOrdering]") {
    Queue q(identity);
    q.enqueue(10);
    q.enqueue(20);
    q.enqueue(30);
    REQUIRE(*q.dequeue() == 10);
    REQUIRE(*q.dequeue() == 20);
    REQUIRE(*q.dequeue() == 30);
}

TEST_CASE("GroupOrdering: three groups dequeue in group-creation order", "[GroupOrdering]") {
    Queue q(mod3);
    q.enqueue(1);  // group key=1
    q.enqueue(2);  // group key=2
    q.enqueue(0);  // group key=0
    q.enqueue(4);  // key=1, joins first group
    q.enqueue(5);  // key=2, joins second group
    // Expected: 1,4 | 2,5 | 0
    REQUIRE(*q.dequeue() == 1);
    REQUIRE(*q.dequeue() == 4);
    REQUIRE(*q.dequeue() == 2);
    REQUIRE(*q.dequeue() == 5);
    REQUIRE(*q.dequeue() == 0);
}

TEST_CASE("GroupOrdering: late arrival does not displace other groups", "[GroupOrdering]") {
    Queue q(mod3);
    q.enqueue(1);  // group key=1
    q.enqueue(2);  // group key=2
    q.enqueue(4);  // key=1, joins first group — group 2 position unchanged
    // group 2 is still second
    q.dequeue();   // 1
    q.dequeue();   // 4
    REQUIRE(*q.dequeue() == 2);
}

// Interleaving tests

TEST_CASE("Interleaving: size tracks correctly across mixed ops", "[Interleaving]") {
    Queue q(mod3);
    q.enqueue(1);
    REQUIRE(q.size() == 1);
    q.enqueue(2);
    REQUIRE(q.size() == 2);
    q.dequeue();
    REQUIRE(q.size() == 1);
    q.enqueue(4);
    REQUIRE(q.size() == 2);
    q.dequeue();
    q.dequeue();
    REQUIRE(q.size() == 0);
    REQUIRE(q.empty());
}

TEST_CASE("Interleaving: alternating enqueue/dequeue matches expected sequence", "[Interleaving]") {
    Queue q(mod3);
    q.enqueue(3);  // key 0
    q.enqueue(1);  // key 1
    q.enqueue(6);  // key 0 -> joins group of 3
    REQUIRE(*q.dequeue() == 3);
    q.enqueue(9);  // key 0 -> joins group of 6 (group still alive)
    REQUIRE(*q.dequeue() == 6);
    REQUIRE(*q.dequeue() == 9);
    REQUIRE(*q.dequeue() == 1);
    REQUIRE(q.empty());
}

TEST_CASE("Interleaving: dequeue between enqueues maintains invariants", "[Interleaving]") {
    Queue q(mod3);
    for (int i = 0; i < 9; ++i) q.enqueue(i);
    // Groups: key0=[0,3,6], key1=[1,4,7], key2=[2,5,8]
    REQUIRE(*q.dequeue() == 0);
    REQUIRE(*q.dequeue() == 3);
    REQUIRE(*q.dequeue() == 6);  // group 0 drained
    REQUIRE(*q.dequeue() == 1);
    q.enqueue(10);               // key 1 — group 1 still alive, joins it
    REQUIRE(*q.dequeue() == 4);
    REQUIRE(*q.dequeue() == 7);
    REQUIRE(*q.dequeue() == 10);
    REQUIRE(q.size() == 3);      // group 2 remains: [2,5,8]
}
