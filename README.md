# Israeli Queue

A generic, header-only C++17 implementation of the Israeli Queue data structure.

An Israeli Queue is a queue in which each arriving element scans for a known peer already waiting. If found, it joins the tail of that peer's group rather than the global back. All members of a group are served consecutively. If no known peer exists, the element starts a new group at the back.

## Installation

**Copy the header:**
```
include/israeli_queue/israeli_queue.hpp  →  your project's include path
```

**Or via CMake FetchContent** (CMake 3.14+, C++17 required):
```cmake
include(FetchContent)
FetchContent_Declare(
    israeli_queue
    GIT_REPOSITORY https://github.com/NavotYaari/israelic_queue.git
    GIT_TAG        main
)
# Disable the library's own tests and benchmarks
set(IQ_BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(IQ_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(israeli_queue)
target_link_libraries(your_target PRIVATE israeli_queue)
```

## Usage

```cpp
#include <iostream>
#include <string>
#include <israeli_queue/israeli_queue.hpp>

struct Task { int tenant_id; std::string payload; };

// Key extractor: group tasks by tenant
auto key = [](const Task& t) { return t.tenant_id; };

iq::IsraeliQueue<Task, decltype(key)> q(key);

q.enqueue(Task{1, "a"});
q.enqueue(Task{2, "b"});
q.enqueue(Task{1, "c"});  // joins tenant-1 group

// Dequeue order: {1,"a"}, {1,"c"}, {2,"b"}
// A plain FIFO would yield: {1,"a"}, {2,"b"}, {1,"c"}
while (!q.empty()) {
    auto task = q.dequeue();  // returns std::optional<Task>
    std::cout << task->tenant_id << " " << task->payload << "\n";
}
```

### Starvation prevention

Pass a `max_group_size` to cap how many elements can join any one group. Once full, the next same-key element starts a new group at the back, bounding how long any task can be delayed.

```cpp
// Groups cap at 10 elements. A task at the back is served
// within K * 10 dequeues (K = number of active groups ahead).
iq::IsraeliQueue<Task, decltype(key)> q(key, /*max_group_size=*/10);
```

## API

| Operation | Signature | Complexity | On empty |
|---|---|---|---|
| `enqueue` | `void enqueue(T)` | O(1) avg | — |
| `dequeue` | `std::optional<T> dequeue()` | O(1) avg | Returns `std::nullopt` |
| `front` | `const T& front() const` | O(1) | UB — check `empty()` first |
| `pop` | `void pop()` | O(1) avg | UB — check `empty()` first |
| `empty` | `bool empty() const` | O(1) | — |
| `size` | `std::size_t size() const` | O(1) | — |

`dequeue()` is the safe path (check + consume in one call). `front()` + `pop()` is the fast path for callers who have already called `empty()`, matching the `std::queue` convention.

## Complexity

| | Average | Worst case |
|---|---|---|
| `enqueue` | O(1) | O(n) — hash rehash |
| `dequeue` / `pop` | O(1) | O(n) — hash rehash |
| Space | O(n + g) | — |

`n` = total elements, `g` = number of live groups.

## Key type requirements

The type returned by the key extractor must be:
- Hashable via `std::hash`
- Equality-comparable via `std::equal_to`

## Thread safety

Same contract as `std::queue`: concurrent reads are safe; any write requires exclusive external synchronisation. No internal locking.

## When to use

The Israeli Queue delivers its grouping benefit when `1 < K << N` — multiple groups, each with many members.

| Scenario | Recommendation |
|---|---|
| K = 1 (all same key) | Equivalent to plain FIFO with extra hash overhead — use `std::queue` |
| K = N (all unique keys) | Degenerates to plain FIFO with per-element hash cost — use `std::queue` |
| 1 < K << N | Israeli Queue is the right tool |

## Building tests and benchmarks

```bash
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build
ctest --test-dir build        # run tests
./build/benchmarks/israeli_queue_benchmarks  # run benchmarks
```

Requires CMake 3.14+, a C++17 compiler, and internet access on first configure (FetchContent downloads Catch2 and nanobench).

## License

MIT
