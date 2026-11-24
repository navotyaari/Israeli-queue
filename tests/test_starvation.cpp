#include <catch2/catch_test_macros.hpp>
#include <israeli_queue/israeli_queue.hpp>

using Queue    = iq::IsraeliQueue<int, int(*)(const int&)>;
using QueueCap = iq::IsraeliQueue<int, int(*)(const int&)>;

static int mod3(const int& v) { return v % 3; }
static int identity(const int& v) { return v; }

TEST_CASE("StarvationPrevention: default unlimited preserves original behaviour", "[StarvationPrevention]") {
    Queue q(mod3);
    for (int i = 0; i < 9; ++i) q.enqueue(i);
    // Groups: key0=[0,3,6], key1=[1,4,7], key2=[2,5,8]
    REQUIRE(*q.dequeue() == 0);
    REQUIRE(*q.dequeue() == 3);
    REQUIRE(*q.dequeue() == 6);
    REQUIRE(*q.dequeue() == 1);
    REQUIRE(*q.dequeue() == 4);
    REQUIRE(*q.dequeue() == 7);
    REQUIRE(*q.dequeue() == 2);
}

TEST_CASE("StarvationPrevention: full group causes overflow to new back group", "[StarvationPrevention]") {
    QueueCap q(mod3, 2);
    q.enqueue(3);  // key 0, group A, size=1
    q.enqueue(1);  // key 1, group B
    q.enqueue(6);  // key 0, group A, size=2 (full)
    q.enqueue(9);  // key 0, group A full -> overflow group A2 at back
    // Queue: [group A: 3,6] [group B: 1] [group A2: 9]
    REQUIRE(*q.dequeue() == 3);
    REQUIRE(*q.dequeue() == 6);
    REQUIRE(*q.dequeue() == 1);
    REQUIRE(*q.dequeue() == 9);
}

TEST_CASE("StarvationPrevention: index updated to overflow group", "[StarvationPrevention]") {
    QueueCap q(mod3, 2);
    q.enqueue(3);  // key 0, group A, size=1
    q.enqueue(6);  // key 0, group A, size=2 (full)
    q.enqueue(9);  // key 0, overflow -> group A2 at back
    q.enqueue(12); // key 0, joins group A2 (not A, which is full)
    // Queue: [group A: 3,6] [group A2: 9,12]
    REQUIRE(*q.dequeue() == 3);
    REQUIRE(*q.dequeue() == 6);
    REQUIRE(*q.dequeue() == 9);
    REQUIRE(*q.dequeue() == 12);
}

TEST_CASE("StarvationPrevention: full group drains before overflow group", "[StarvationPrevention]") {
    QueueCap q(mod3, 1);
    q.enqueue(3);  // key 0, group A (cap=1, immediately full)
    q.enqueue(6);  // key 0, overflow -> group A2
    q.enqueue(9);  // key 0, overflow -> group A3
    // Queue: [3] [6] [9] — each is its own group
    REQUIRE(*q.dequeue() == 3);
    REQUIRE(*q.dequeue() == 6);
    REQUIRE(*q.dequeue() == 9);
}

TEST_CASE("StarvationPrevention: multiple overflows drain in creation order", "[StarvationPrevention]") {
    QueueCap q(mod3, 2);
    // Enqueue 6 elements with key 0, cap=2 -> three groups of 2
    q.enqueue(3);
    q.enqueue(6);   // group A full
    q.enqueue(9);
    q.enqueue(12);  // group A2 full
    q.enqueue(15);
    q.enqueue(18);  // group A3 full
    REQUIRE(*q.dequeue() == 3);
    REQUIRE(*q.dequeue() == 6);
    REQUIRE(*q.dequeue() == 9);
    REQUIRE(*q.dequeue() == 12);
    REQUIRE(*q.dequeue() == 15);
    REQUIRE(*q.dequeue() == 18);
}

TEST_CASE("StarvationPrevention: max_group_size 1 degrades to FIFO", "[StarvationPrevention]") {
    QueueCap q(identity, 1);
    q.enqueue(10);
    q.enqueue(20);
    q.enqueue(10); // same key as first, but cap=1 -> new group at back
    q.enqueue(30);
    REQUIRE(*q.dequeue() == 10);
    REQUIRE(*q.dequeue() == 20);
    REQUIRE(*q.dequeue() == 10);
    REQUIRE(*q.dequeue() == 30);
}

TEST_CASE("StarvationPrevention: lone task eventually dequeued when capped", "[StarvationPrevention]") {
    QueueCap q(mod3, 2);
    q.enqueue(99); // key 0 (99 % 3 = 0), lone task at back — actually first
    q.enqueue(1);  // key 1
    q.enqueue(4);  // key 1, group fills (cap=2)
    q.enqueue(7);  // key 1, overflow -> new group at back
    // Queue: [99] [1,4] [7]
    REQUIRE(*q.dequeue() == 99);
    REQUIRE(*q.dequeue() == 1);
    REQUIRE(*q.dequeue() == 4);
    REQUIRE(*q.dequeue() == 7);
}
