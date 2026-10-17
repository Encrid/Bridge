# src/schedule_closure.c

This file closes a matrix of `src/schedule_matrix.c`, and finds a system that would run before itself.

## The closure

`schedule_close_matrix` is the closure of Warshall, one system at a time.
For each system `k`, `schedule_join_through` gives each row that reaches `k` the row of `k`.
After it, each system that reaches a system reaches all that system reaches: the matrix is closed.

## A cycle

In a closed matrix, each system on a cycle reaches itself.
`schedule_get_cycle` gives the first system that reaches itself, or the count of rows if no system does.
`src/schedule_order.c` uses it to refuse pairs that contradict each other.
`src/schedule_check.c` uses it to refuse a phase with a cycle.

## Functions

| Function | Result |
| --- | --- |
| `schedule_close_matrix` | Calls `schedule_join_through` for each system: the closure. |
| `schedule_get_cycle` | Gives the first system of a closed matrix that reaches itself, or the count of rows. |
