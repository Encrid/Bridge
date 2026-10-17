# src/schedule_dispatch.c

This file runs one system: it tells the module which system runs, makes the call, and keeps what the call gave.

## The culprit

Before the call, `run_system` writes in the header of the private memory of the module which system runs.
It writes `barriers`, `place`, `system` and `observer`. Each line that the module then writes to its log carries them.
Two systems that conflict never run at the same time.
So at one time only one system can write a given component, and the header names it: it is the culprit of a bad write.

`system` is the slot of the system among the systems of its module: the number that the module got when it registered the system.
`run_system` takes it from `slot` of the system.
For an observer, it is the slot among the observers, and `observer` is 1.
After the call, `system` is `UINT32_MAX` and `observer` is 0.
While the system runs, `running` and `turn` of the module hold the system and its turn.
The entries that the system calls read them: `src/abi_calls.c` checks the stated changes, and `src/abi_entry.h` marks the turn in the queue.

## The work limit

What is left of a work limit only goes down, so `call_system` sets the limit again before every call.
It does this only if `work_limit` of the module is above 0: `-1` is no limit, and sets nothing.

## A failed call

`call_system` marks the system `failed`, with the reason in `why`, in three cases:

| Case | Reason |
| --- | --- |
| The call did not return, for example a trap. | The text of the error of the backend. |
| An entry refused a request during the call, and set `refused` of the module. | The reason of the module, in `why` of the module. |
| The call returned a status that is not 0. | That it answered the status. |

`src/schedule_run.c` counts the failure after the level ends.

## Functions

| Function | Result |
| --- | --- |
| `call_system` | Sets the work limit, calls the system, and marks it failed with a reason if the call failed. |
| `run_system` | Writes which system runs in the header of the module, runs it, then clears the header. |
| `schedule_run_job` | What a thread runs: the systems of one job, in the order the module registered them. |
