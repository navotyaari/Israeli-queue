# Assignment: Israeli Queue Library

## Description

Implement a library for an **Israeli Queue** data structure.

An Israeli Queue is a queue in which each arriving customer (task) checks whether it
knows any customer already waiting in the queue. If it does, it joins the back of that
customer's group. If it does not know anyone, it joins the back of the queue.

All members of the same group are served consecutively — the entire group is treated
as a contiguous block within the queue.

## Functional Requirements

- Customers are enqueued one at a time.
- On enqueue, the new customer scans the queue for a known customer.
- If a known customer is found, the newcomer joins at the end of that customer's group.
- If no known customer is found, the newcomer joins at the back of the queue.
- Customers are dequeued from the front of the queue, one at a time.
- The "knows" relationship must be defined by the caller (exact mechanism TBD).

## Non-Functional Requirements

- The library should be generic — it must support any customer/task type.
- Enqueue and dequeue operations should be efficient (target O(1) average).
- The library should be usable as a standalone module/package.

## Programming Language

To be decided.

## Time Budget

None.
