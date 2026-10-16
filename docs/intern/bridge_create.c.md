# src/bridge_create.c

This file makes a deck of a deployment: it opens the backends, names the phases, starts the threads and builds the storage.
A deck loads no module: the host loads each module after `bridge_create`.

## The parts of a deck

1. Read the number of the host seam, then make room for the backends and the phases.
2. Open and start the backends: `src/bridge_backends.c`.
3. Take the phases: `src/bridge_phases.c`.
4. Take the pairs of `[order]`: `src/schedule_order.c`.
5. Start the threads that run a phase.
6. Make the shared block and the storage in it: `src/block_create.c`.
7. Make what a dispatch and a schedule are worked out in: `src/schedule_account.c`.

## When create fails

`bridge_create` gives null only when the deployment cannot start.
A record to the sink tells the reason, and `bridge_destroy` frees what was made.
These are the reasons:

- The sink or the deployment is null. No record can tell this, and a host that passed null knows it already.
- There is no memory for the deck. Then a record is made without a deck, and handed to the sink.
- The number of the host seam is unset, or is not `BRIDGE_HOST_ABI`.
- A key of the deployment is unset, for example `threads`.
- The deployment has no backend, or no phase.
- A backend or a phase has no name, or has the name of another.
- A pair of `[order]` says nothing, or contradicts another pair.
- A budget cannot be satisfied: `docs/intern/block_budget.c.md`, *The budget of a deployment*.
- The library of a backend does not open or lacks an entry, or the backend does not start.

## The threads

`threads` is the number of threads that run a phase, and it counts the thread that calls `bridge_run_phase`.
Bridge does not guess it, so an unset `threads` is refused.
The pool has `threads - 1` workers, and room for `[budget] systems` jobs.
A job is the systems of one module in one level, so there are never more jobs than systems.

## The numbers of the seams

Each of the three seams has a number: the host seam, the backend seam and the module seam.
A host, a backend and a module are each built at their own time, so a side can state another number.
A struct read at offsets that the other side does not use is an error that nobody would see.
So Bridge refuses a side that states another number, before it reads more of that side.
Each number is first: in the struct, or in the entries.

The host states `BRIDGE_HOST_ABI` first in `bridge_config_t`, and `bridge_create` refuses another number, with a record that tells both numbers.
`src/backend.c` checks the backend seam, and `src/module_check.c` checks the module seam.
All three numbers are 1 until there is a release: a tagged version given to something that is not built again with it.
After the first release, a change to what a seam lays out or means increments the number of that seam, and only that number.

## Functions

| Function | Result |
| --- | --- |
| `make_bridge` | Gives an empty deck that knows its sink. If there is not the memory, it hands the sink a record of its own and gives null. |
| `check_config` | Refuses a number of the host seam that is unset or another, then makes room for the backends and the phases. 1, with a record, if refused. |
| `start_pool` | Refuses an unset `threads`, and starts the threads that run a phase. 1, with a record, if refused. |
| `build_bridge` | Makes each part of the deck that the deployment states. 1, with a record, at the first part that is refused. |
| `bridge_create` | Makes a deck of a deployment. Gives the deck, or null if the deployment cannot start. |
