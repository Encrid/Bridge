# src/backend.h

This header gives `backend_open`, which opens the library of a backend and finds its entries.
The types of the backend seam, `backend_api_t` among them, are in `include/bridge_backend.h`.

## The entries of a backend

A backend is a library of its own that runs the modules of a world.
Bridge opens the library by the path that the deployment states, and finds its eighteen entries by name.
`backend_api_t` holds the entries in the order of `BACKEND_ENTRY_NAMES`.
The first entry gives the number of the backend seam that the library was built against.
Bridge asks a library of another number for no other entry.

| Entry | What Bridge asks for |
| --- | --- |
| `backend_get_abi` | The number of the backend seam. |
| `backend_create`, `backend_destroy` | A backend started with its settings, which Bridge passes unread, and the backend stopped. |
| `backend_get_capabilities` | The guarantees that the backend can keep, as flags. |
| `memory_get_shape` | What the backend needs of a block that it maps: a reservation, a guard and a granule. |
| `memory_adopt`, `memory_release` | A block of Bridge made addressable by modules, and the handle given back. |
| `module_compile`, `module_destroy` | The image of a module, made from its bytes or its path, and the image destroyed. |
| `module_list_imports` | The imports of an image, so that an instance gets only the memories that it imports. |
| `instance_create`, `instance_destroy` | An instance of an image, with its memories, and the instance destroyed. |
| `instance_set_work_limit`, `instance_set_timeout`, `instance_set_memory_limit` | The limits of an instance. |
| `define_func` | One entry of the ABI, defined for an instance with its signature. |
| `instance_get_func` | An export that `call` can run: it takes nothing and gives a status. |
| `call` | One call of an export: whether it returned, and the status that it gave. |

An image is the loaded form of a module: compiled bytes on the wasm backend, an opened library on the native backend.
`module_compile` gets both the bytes and the path, because Bridge reads neither of them.
Either can be null. A backend that needs the one that it did not get refuses the module by name.
That refusal, and a library that the operating system cannot open, are failures of the module, as a failed `init` is.

`module_compile` also gets `demands`: the capabilities that the config of the module asks for.
A backend need not compile in a guarantee that nobody asked for.
A backend can keep more than was asked, but never less.

A block is adopted, not created: the backend makes addressable a block that Bridge allocated.
Bridge releases a memory only after it destroys the last instance that it gave the memory to.
`instance_set_timeout` takes nanoseconds, for the next call only, and the backend changes them to the unit of its engine.
Only a lifecycle call gets a timeout. A system gets a work limit, which Bridge sets again before each dispatch.
A backend does not decide when anything runs, find a file, or know what a component, an entity or a barrier is.

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `backend_open` | `src/backend.c` | Opens the library of a backend and finds its eighteen entries by name. Gives the library, or null with `err` that tells why. |
