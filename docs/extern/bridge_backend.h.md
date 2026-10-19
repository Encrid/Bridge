# include/bridge_backend.h

`include/bridge_backend.h` is the backend seam: what Bridge asks of a backend, the library that runs modules.
Only Bridge and a backend compile it.

## The library

A backend is a shared library that exports the 18 functions of `backend_api_t`, by their names.
`BACKEND_ENTRY_NAMES` gives the names, in the order of the fields of `backend_api_t`.
`BACKEND_ENTRY_COUNT` is how many there are: 18.
`BACKEND_EXPORT` exports a function from the library: a backend in C writes it before each of the 18 functions.
A C++ compiler reads the declarations of this header as C, as it does those of `include/bridge.h`: `docs/extern/bridge.h.md`, *The library*.
A backend in C++ writes `extern "C"` before `BACKEND_EXPORT` on each of the 18 functions, since the header does not declare them.

`bridge_create` opens the library that the deployment names, and finds each entry by its name: `docs/extern/bridge.h.md`, *bridge_backend_t*.
A library that lacks an entry is not a backend, and `bridge_create` refuses the world.
The structs follow the rules of the host seam: `docs/extern/bridge.h.md`, *The layouts*.
A backend is a work based on Bridge, since it compiles with this header: `readme.md`, *License*.

The backend owns five kinds of handle, and Bridge never reads into them:

| Type | Handle |
| --- | --- |
| `backend_t` | The backend that `backend_create` made. |
| `backend_memory_t` | A memory that the backend adopted. |
| `backend_module_t` | The image of a module: compiled bytes, or an opened library. |
| `backend_instance_t` | An instance of an image: what runs. |
| `backend_func_t` | A function of an instance that `call` runs. |

## BACKEND_ABI

`BACKEND_ABI` is the number of the backend seam: the version of its layouts and its entries.
`backend_get_abi` gives the number that the backend was built against.
Bridge calls `backend_get_abi` first, and refuses a library that gives another number, before it asks for another entry.
The number changes as `BRIDGE_HOST_ABI` does: `docs/extern/bridge.h.md`, *BRIDGE_HOST_ABI*.

## backend_err_t

`backend_err_t` is why an entry failed: a text of at most 255 bytes, and a zero after it, in `text`.
The caller owns it: Bridge gives one to each entry that can fail, and the backend writes the text into it.
Bridge puts the text into a record for the host: `docs/extern/bridge.h.md`, *Messages*.

## backend_create

`backend_create` starts the backend with its settings, `count` of them, and gives the backend.
It gives null, and writes `err`, if the backend cannot start.
`backend_destroy` destroys the backend `b`.
Bridge calls it after it destroyed each instance and image of the backend, and released each memory.

A setting of a backend is a `backend_setting_t`, with a value of one of four kinds:

| Field | Content |
| --- | --- |
| `key` | The name of the setting. |
| `kind` | The kind of its value: an item of `enum backend_setting_kind`. |
| `pad` | 7 bytes of padding. |
| `v` | The value: a `backend_value_t`. |

| `kind` | Field of `v` | Value |
| --- | --- | --- |
| `BACKEND_BOOL` | `b` | A `bool`. |
| `BACKEND_INT` | `i` | An `int64_t`. |
| `BACKEND_FLOAT` | `f` | A `double`. |
| `BACKEND_STRING` | `s` | A text that ends with a zero. |

The settings are the own settings of the backend, for example a folder for a cache. Bridge gives them unread.
Refuse a key that the backend does not know, by its name: a key that nobody reads is a fault that nobody finds.
`backend_setting_t` has the shape of `bridge_setting_t` of the host seam: `docs/extern/bridge.h.md`, *Settings*.
Each seam header is whole without the other. Bridge asserts that the two have one shape, where it copies one to the other.

## backend_capability

`backend_get_capabilities` gives what the backend can do: a mask of items of `enum backend_capability`.
Each capability is a promise. State only what the backend keeps.

| Item | Promise | The key that demands it |
| --- | --- | --- |
| `BACKEND_UNTRUSTED` | The backend runs a module that is not trusted. | `trusted` of `BRIDGE_FALSE` |
| `BACKEND_WORK_LIMIT` | The backend counts the work of an invocation, and stops it at its limit. | `work_limit` |
| `BACKEND_TIMEOUT` | The backend stops a call at its timeout. | A timeout |
| `BACKEND_MEMORY_LIMIT` | The backend keeps the own memory of a module under a ceiling. | `memory_limit` |
| `BACKEND_RELOAD` | A reload can replace a module while the world runs. | `hot_reload` |
| `BACKEND_SIDE_BY_SIDE` | Two instances of one module can exist at one time. | None. |
| `BACKEND_UNLOAD` | Bridge can take the library out of the process after `backend_destroy`. | None. |

A module that is not trusted cannot reach memory that it was not given.
Such a backend checks each export that it gives: `call` can run it. How the backend does it is its own business.

If the backend does not state a capability that the config of a module demands, a load refuses the module: `docs/extern/bridge.h.md`, *The limits*.
`BACKEND_SIDE_BY_SIDE` decides the order of a reload: `docs/extern/bridge.h.md`, *bridge_reload*.
A backend that brought a runtime with threads of its own into the process does not state `BACKEND_UNLOAD`.
Bridge reads the capabilities and the shape of a backend once, after `backend_create`.

## Memory

`memory_get_shape` tells what the backend needs of a block of memory that it maps.
It writes three numbers, and 0 for each one that it does not need:

| Parameter | Content |
| --- | --- |
| `reservation` | How many bytes of address space, from the start of the block, are reserved for the block. |
| `guard` | How many bytes after the reservation have nothing mapped into them. |
| `granule` | The length of the block is a whole number of `granule` bytes. |

A `granule` that is not 0 is a power of two, of at most 1073741824 bytes.
A `reservation` and a `guard` each have at most 140737488355328 bytes, 2 to the power 47.
Bridge refuses a backend that states more, and the world with it.

Bridge reserves the length of a block or `reservation`, the larger of the two, and `guard` after it.
Each backend maps the shared memory, thus it has the largest `reservation` and `guard`, and a length that is a whole number of each `granule`.
The private memory of a module has the shape that the backend of the module needs.

`memory_adopt` makes `len` bytes at `ptr`, which Bridge owns, a memory that the modules of the backend can address.
It does not allocate. It gives the handle of the memory, or null with `err`.
A backend that refuses the shared memory refuses the world. A backend that refuses a private memory fails the load of its module.

`memory_release` gives the handle back, and frees nothing of the block.
Bridge calls it after it destroyed the last instance that it gave the memory to.

`backend_memory_import_t` tells which memory answers which import of a module:

| Field | Content |
| --- | --- |
| `ns` | The namespace of the import: `bridge`. |
| `field` | The name of the import: `shared` or `private`. |
| `memory` | The adopted memory that answers the import. |

## module_compile

`module_compile` makes the image of a module from a `backend_source_t`: for example, it compiles bytes or opens a library.
It gives the image, or null with `err`.

| Field | Content |
| --- | --- |
| `bytes` | The image of the module, as the host gave it: `docs/extern/bridge.h.md`, *bridge_artifact_t*. |
| `len` | How many bytes `bytes` holds. |
| `path` | The path of the module, or null. |
| `demands` | The capabilities that the config of the module demands: a mask of items of `enum backend_capability`. |
| `pad` | 4 bytes of padding. |

Bridge gives the bytes and the path of the host unread. A backend uses the one that it needs.
A backend that needs one of them, and gets null, refuses the module and writes why.
A backend can leave out a guarantee that nobody demands, for example the count of a work limit.
A backend can enforce more than `demands`, and never less.

`module_list_imports` gives how many imports the image makes, and fills at most `max` items of `out`.
Each item is a `backend_import_t`.
Bridge reads the imports to know which memories to give to `instance_create`. It gives a `max` of 64.

| Field | Content |
| --- | --- |
| `ns` | The namespace of the import. A zero does not follow it. |
| `ns_len` | How many bytes `ns` has. |
| `field` | The name of the import. A zero does not follow it. |
| `field_len` | How many bytes `field` has. |
| `kind` | What the import is: an item of `enum backend_import_kind`. |
| `pad` | 7 bytes of padding. |

| Item | The import is |
| --- | --- |
| `BACKEND_IMPORT_FUNC` | A function. |
| `BACKEND_IMPORT_MEMORY` | A memory. |
| `BACKEND_IMPORT_OTHER` | Another thing. |

`module_destroy` destroys an image. Bridge calls it after it destroyed each instance of the image.

## Instances

`instance_create` makes an instance of the image `m`: the unit that runs.
It gets the memories that the module imports, `count` items of `backend_memory_import_t`, and gives the instance, or null with `err`.
An instance is of the backend of its image. Bridge calls an instance from one thread at a time.
`instance_destroy` destroys an instance.

| Entry | What it sets |
| --- | --- |
| `instance_set_work_limit` | The work that the next invocation can do, in `units` of the backend. |
| `instance_set_timeout` | The timeout of the next call only, in `nanos` of real time. |
| `instance_set_memory_limit` | The ceiling of the own memory of the module, in `bytes`. |

Bridge sets the work limit before each invocation of a system, since what is left of a limit only goes down.
Bridge sets the timeout before each call of `init`, `reload` and `shutdown`. The backend converts it to the unit of its clock.
Bridge sets the memory limit one time, after it made the instance.
Bridge calls each of the three only for a limit above zero, and only on a backend with the capability.

## define_func

`define_func` defines an import that a module can make, and the function that answers it: a `backend_entry_t`.
It gives 0, or a value that is not 0, with `err`.

| Field | Content |
| --- | --- |
| `ns` | The namespace of the import. |
| `field` | The name of the import. |
| `fn` | The function that answers the import: a `backend_fn_t`. |
| `ud` | The pointer that the backend gives to `fn` at each call. |
| `sig` | The signature of the function: a `backend_sig_t`. |

Bridge defines the eight entries of the module seam, in the namespace `bridge`: `docs/extern/bridge_abi.h.md`, *bridge_entries_t*.
Bridge defines each import before it asks for the first export.
Bridge states the signature of each import. The backend checks the shape of each export.

| Field | Content |
| --- | --- |
| `params` | The types of the parameters, `param_count` of them. |
| `param_count` | How many parameters the function has. |
| `pad0` | 4 bytes of padding. |
| `results` | The types of the results, `result_count` of them. |
| `result_count` | How many results the function has. |
| `pad1` | 4 bytes of padding. |

| Item | Type |
| --- | --- |
| `BACKEND_I32` | An integer of 32 bits. |
| `BACKEND_I64` | An integer of 64 bits. |

No entry takes or gives a float.
The backend calls `fn` with `ud`, the parameters in `args`, and room for the results in `results`: each value in 64 bits.
`fn` gives 0, or a value that is not 0: then the call that reached it must not go on.
A backend that can stop the module stops it there. Another backend lets the call return, and Bridge fails the invocation then.

## instance_get_func

`instance_get_func` gives a function that `call` can run: an export of the instance, by its `name`.
The export takes nothing, and gives an `int32_t` status.
It gives null, with `err`, if there is no export of that name, or if the backend sees that the export has another shape.
At each load and reload, Bridge asks for `init` or `reload`, for each system and observer, and for `shutdown` and `reload`.
A backend that cannot see the type of an export checks nothing, and does not state `BACKEND_UNTRUSTED`.

`call` runs a function. It gives 0 if the function returned, and then `status` is what the function gave.
It gives a value that is not 0, with `err`, if the function did not return.
For example, the function met its timeout or its work limit, or read out of its memory. The process goes on.
Bridge treats a call that did not return as it treats a status that is not 0: `docs/extern/bridge_abi.h.md`, *The exports*.

## The order of the calls

| When | Bridge calls |
| --- | --- |
| `bridge_create` | `backend_get_abi`, `backend_create`, `backend_get_capabilities`, `memory_get_shape`, then `memory_adopt` of the shared memory. |
| A load | `module_compile`, `memory_adopt` of the private memory, `module_list_imports`, `instance_create`, and `define_func` 8 times. |
| A load, then | `instance_set_memory_limit`, `instance_get_func` of `init`, `instance_set_timeout`, `call` of `init`, then `instance_get_func` of each export. |
| A phase | `instance_set_work_limit` and `call` of each system. |
| An unload | `instance_set_timeout`, `call` of `shutdown`, `instance_destroy`, `module_destroy` and `memory_release`. |
| `bridge_destroy` | The `shutdown` of each module, the destruction of each module, `memory_release` of the shared memory, then `backend_destroy`. |

A reload calls as a load does for the new instance, with `reload` in place of `init`.
For the old instance, it calls as an unload does.

## Messages

`bridge_create` writes one of these reasons when a library is not a backend.
The record is `backend '<backend>': <reason>`: `docs/extern/bridge.h.md`, *The deployment*.
Angle brackets mark a part that changes: `docs/extern/bridge.h.md`, *Messages*.

- `backend library '<path>' has no entry '<entry>'`
- `backend library '<path>' is of backend ABI <number>, and this is <number>`

Bridge puts each text that a backend writes in `err` into a record of its own, after the name of the backend or the module.

## The native backend

The native backend, `native`, is in `src/backends/native`. It runs each module as a native shared library.
It opens the library of a module from the path that the host gave, and finds each export by its name, as `bridge_<name>`.
The native backend keeps none of the limits, and runs only a module that is trusted.

It writes one of these reasons in `err` when it refuses. Angle brackets mark a part that changes:

- `it knows no setting '<key>'`
- `the native SDK has no entry '<name>'`
- `no exported function '<name>'`
- `this backend opens a module's library by its path, and was handed none`
- `cannot open the library '<path>'`

## The wasm backend

The wasm backend, `wasm`, is in `src/backends/wasm`. It runs each module as sandboxed core wasm over
wasmtime, which it fetches and pins. It compiles the bytes of a module into a wasm instance, and reaches
each export by its name.

It writes this reason in `err` when an export has the wrong type. The angle brackets mark a part that
changes:

- `its export '<name>' is not () -> i32`
