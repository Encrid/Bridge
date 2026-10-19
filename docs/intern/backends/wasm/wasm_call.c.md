# src/backends/wasm/wasm_call.c

This file sets a call's work limit and timeout, and runs an exported function.
A timed call arms the watchdog before it runs and disarms it after.

## Arming a call

`instance_set_work_limit` sets the fuel for a metered module.
`instance_set_timeout` keeps the deadline for the next call.
`arm` sets the engine's epoch deadline, records the call under the lock, and wakes the watchdog.
`disarm` clears the active call and wakes the watchdog again.
`call` runs the function with no argument, reads its one `i32` status, and reports a trap through `err`.

## Functions

| Function | Result |
| --- | --- |
| `instance_set_work_limit` | Sets the fuel a metered call may spend. |
| `instance_set_timeout` | Keeps the nanoseconds the next call may run. |
| `call` | Runs an exported function and reports its `i32` status, or -1 with `err` on a trap. |
