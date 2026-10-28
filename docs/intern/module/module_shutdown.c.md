# src/module/module_shutdown.c

This file runs the `shutdown` of a module: of a loaded module, and of a new instance whose load or reload fails.

## A loaded module

`module_shut_down` runs the `shutdown` of a module under its `shutdown_timeout`.
A `shutdown` that fails is said as an error, and the teardown continues.
Then storage applies what the module asked for, and the log ring of the module is drained, since the ring goes with the module.
A module whose reload failed after its old instance went has no code left, so Bridge does not call it.

## A new instance that fails

`module_shut_down_new` runs the `shutdown` of a new instance when its load or its reload fails. The list of modules does not hold such an instance.
It runs it only if the first call of the instance gave 0, and if the instance has a `shutdown`.
The record holds `shutdown` only from that point: `docs/intern/module/module_start.c.md`, *The exports*. Thus one test of `shutdown` tells both.
The call is the call of a loaded module: under the `shutdown_timeout` of its config, with the same error if it fails, which stops nothing.

Nothing that the instance queued is applied, also what its `shutdown` queued.
Storage applies only the queues of the modules in the list, and the queue of the instance goes with its record.
`module_shut_down_new` applies no queue of another module either. Thus it changes nothing in storage.
Then the log ring of the instance is drained, so each line that it wrote is said before the ring goes.

## Functions

| Function | Result |
| --- | --- |
| `call_shutdown` | Calls the `shutdown` of a module once, under its `shutdown_timeout`. Says a failure as an error. |
| `module_shut_down` | Runs the `shutdown` of a module, ends the hold on the ids handed out, applies what it asked for, and drains what it said. |
| `module_shut_down_new` | Runs the `shutdown` of a new instance whose first call gave 0, if it has one, and drains what it said. Applies nothing. |
