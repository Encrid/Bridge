# src/host/config_check.c

This file checks every statement of every file and counts the refusals.

## The checks

`config_check` walks each file and prints `refused: <file>:<line>: <key>: <reason>` for a bad statement.
`check_entry` sends a statement to the checker of its top name: module, content, order, budget, backend, or the world.
`check_header` refuses a table header that holds nothing.
`check_order` requires that an order key is `order.before` with a list of pairs.

## Functions

| Function | Result |
| --- | --- |
| `config_check` | Checks all files and gives the count of refused statements. |
