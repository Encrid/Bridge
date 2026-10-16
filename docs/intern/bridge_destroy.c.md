# src/bridge_destroy.c

This file unmakes a world: it shuts down each loaded module, and then gives back everything else.

## The steps of destroy

1. Refuse the call if a loaded module is pinned to another thread.
2. Run the `shutdown` of each module, in the order that the modules loaded.
3. Discard each module.
4. Free the schedule, the shared block with the storage, the threads, and the names of the deployment.
5. Destroy each backend, and close the library of each backend that states `BACKEND_UNLOAD`.
6. Free the world.

`bridge_destroy` of a null world does nothing and gives 0.

## The shutdown of the modules

Each `shutdown` runs under the timeout of its module.
A `shutdown` that fails is said in a record, and the other modules shut down all the same.
The modules shut down in the order that they loaded. No module depends on another, so no other order means more.

## A pinned module

`bridge_destroy` gives a status, because one thing can refuse it.
The `shutdown` of a pinned module is code of that module, so it runs on the thread that loaded the module, or not at all.
From another thread, `bridge_destroy` gives 1, with a record that names the module, and touches nothing.
The world stays whole for a call from the thread that can destroy it.
`docs/intern/bridge_run.c.md`, *A pinned module*, tells the rule.

## The libraries of the backends

A backend states `BACKEND_UNLOAD` if its library can go from the process after `backend_destroy`.
Bridge closes only such a library, and the operating system then decides if it unmaps it.
A backend that brought in a runtime with threads of its own does not state it, and its library stays in the process.
The next world finds that library there again.
A library that opened but never became a backend states nothing, and stays too.

## Functions

| Function | Result |
| --- | --- |
| `end_modules` | Runs the `shutdown` of each module in load order, then discards each module. |
| `free_schedule` | Frees the list of systems, and what a dispatch and a schedule are worked out in. |
| `free_names` | Frees the names that the deployment gave: the pairs of `[order]`, and the phases. |
| `close_backends` | Destroys each backend, and closes each library whose backend states `BACKEND_UNLOAD`. |
| `bridge_destroy` | Runs the `shutdown` of each loaded module, then frees the world. 0 if the world is gone, 1 if a pinned module refuses the call. |
