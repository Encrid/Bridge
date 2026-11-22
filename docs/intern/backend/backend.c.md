# src/backend/backend.c

This file opens the library of a backend and finds its 19 entries by name.
It gives `backend_open` and `backend_close`, which `include/bridge_backend.h` declares and the libraries of Bridge export: `docs/extern/bridge_backend.h.md`, *backend_open*.
The types of the backend seam, `backend_api_t` among them, are in `include/bridge_backend.h`.

## The entries of a backend

A backend is a library of its own that runs the modules of a world.
Bridge opens the library by its path in the deployment: `docs/intern/bridge/bridge_backend.c.md`, *A backend of the deployment*.
*The steps* tells how Bridge finds the entries and checks the number of the backend seam.
`backend_api_t` holds the entries in the order of the names of `BACKEND_ENTRY_NAMES`.

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
| `instance_set_work_limit`, `instance_set_timeout`, `instance_set_memory_limit`, `instance_set_element_limit` | The limits of an instance. |
| `define_func` | One entry of the ABI, defined for an instance with its signature. |
| `instance_get_func` | An export that `call` can run: it takes nothing and gives a status. |
| `call` | One call of an export: whether it returned, and the status that it gave. |

An image is the loaded form of a module: compiled bytes on the wasm backend, an opened library on the native backend.
`module_compile` gets both the bytes and the path, because Bridge reads neither of them.
Either can be null. A backend that needs the one that it did not get refuses the module by name.
That refusal, and a library that the operating system cannot open, are failures of the module, as a failed `init` is.

`module_compile` also gets `demands`: `docs/extern/bridge_backend.h.md`, *module_compile*, tells what a backend may do with them.

A block is adopted, not created: the backend makes addressable a block that Bridge allocated.
`docs/extern/bridge_backend.h.md`, *Memory*, tells when Bridge releases a memory.
`instance_set_timeout` takes nanoseconds, for the next call only, and the backend changes them to the unit of its engine.
Only a lifecycle call gets a timeout. A system gets a work limit, which Bridge sets again before each dispatch.
A backend does not decide when anything runs, find a file, or know what a component, an entity or a barrier is.

## The steps

1. Open the library with `os_lib_open`. If it does not open, refuse it by its path.
2. Find each entry by its name, in the order of `ENTRY_NAMES`, the array that this file makes from `BACKEND_ENTRY_NAMES`.
3. After the first entry, compare the number that it gives with `BACKEND_ABI`.
4. If an entry is missing or the number is another, write why in `err`, and close the library.

`find_entries` fills `backend_api_t` as an array, because each member of the struct is a function pointer.

## The number of the backend seam

A library gives the number of the backend seam that it was built against from its first entry, `backend_get_abi`.
A library of another number is not asked for the other entries.
`docs/intern/bridge/bridge_create.c.md`, *The numbers of the seams*, tells why each seam has a number.

## Functions

| Function | Result |
| --- | --- |
| `find_entries` | Fills `api` with the entries of the library. Gives how many it took: all of them, or the index of the entry where it stopped. |
| `say_stopped` | Writes in `err` why `find_entries` stopped: the library lacks the entry, or the first entry gave another number. |
| `backend_open` | Opens the library of a backend and finds its entries. Gives the library, or null with `err` that tells why, and the library closed. |
| `backend_close` | Closes a library that `backend_open` gave, with `os_lib_close`. |
