# src/schedule/schedule_dispatch.c

This file runs one system: it tells the module which system runs, makes the call, and keeps what the call gave.

## The culprit

Before the call, `run_system` writes in the header of the private memory of the module which system runs.
It writes `barriers`, `place`, `system` and `observer`. Each line that the module then writes to its log carries them.
Two systems that conflict never run at the same time.
So at one time only one system can write a given component, and the header names it: it is the culprit of a bad write.

`system` is the slot of the system, which the module got when it registered the system: `docs/intern/abi/abi_subscribe.c.md`, *The slot*.
`run_system` takes it from `slot` of the system.
For an observer, it is the slot among the observers, and `observer` is 1.
After the call, `system` is `UINT32_MAX` and `observer` is 0.
While the system runs, `running` and `turn` of the module hold the system and its turn.
The entries that the system calls read them: `src/abi/abi_calls.c` checks the stated changes, and `src/abi/abi_entry.h` marks the turn in the queue.

## The work limit

What is left of a work limit only goes down, so `call_system` sets the limit again before every call.
It does this only if `work_limit` of the module is above 0: `-1` is no limit, and sets nothing.

## A failed call

`call_system` marks the system `failed`, with the reason in `why`, in three cases:

| Case | Reason |
| --- | --- |
| The call did not return, for example a trap. | The text of the error of the backend. |
| An entry refused a request during the call, and set `refused` of the module. | The reason of the module, in `why` of the module. |
| The call returned a status that is neither 0 nor `BRIDGE_END`. | That it answered the status. |

`src/schedule/schedule_run.c` counts the failure after the level ends.

## The end of the run

A call that returns `BRIDGE_END` is no failure: `call_system` sets `ended` of the system, and gives no reason.
The thread of the job only marks the system. The thread that asked for the phase writes the record after the level.
`docs/intern/schedule/schedule_account.c.md`, *The end of the run* tells that step.

## The time

When `profile` of the world is on, `run_system` reads the clock before and after the call, and adds the span to `nanos` of the system.
It adds 1 to `calls` for an observer, since a system runs once a pass and its count would say nothing.
When `profile` is off, it reads no clock and writes nothing: the cost is one test.
`src/bridge/bridge_profile.c` gives the counts to a host.

## The check

When `validate` of the world is on, `after_call` checks the columns after the call: `src/schedule/schedule_validate.c`.
It names a system whose write no term of it declared, as *The culprit* tells.
When `validate` is off, it makes no check, and the cost is one test.

## Functions

| Function | Result |
| --- | --- |
| `call_system` | Sets the work limit, calls the system, and marks it failed with a reason, or ended, by what the call gave. |
| `after_call` | Times the run when `profile` is on, and checks the columns when `validate` is on. |
| `run_system` | Writes which system runs in the header, calls it, lets `after_call` run the diagnostics, then clears the header. |
| `schedule_run_job` | What a thread runs: the systems of one job, in the order the module registered them. |
