# include/bridge.h

`include/bridge.h` is the only header that a host needs.
It gives the seven functions of the library, and the structs that a host gives to them and gets from them.

## The library

A host links the built library: the shared library or the static library.
The shared library exports only the seven functions of this header, and `BRIDGE_API` marks each of them.
Only the build of the library defines `BRIDGE_BUILDING`. A host does not define it.
A C++ compiler reads the declarations as C, thus a host in C++ links the names as the library exports them.
What a host compiles with this header is a work based on Bridge: `readme.md`, *License*.

A host gives values in structs, and Bridge reads no file of the host.
Bridge has no default value: the host states each value. A field left at zero is unset, and Bridge refuses it by its name.
A count of an array can be zero, unless this document tells otherwise.
Each function but `bridge_list_modules` is `[[nodiscard]]`: the compiler warns when a host ignores its result.
Call `bridge_load`, `bridge_unload` and `bridge_reload` between two phases: never while `bridge_run_phase` runs.

A call from the sink gives 1: *bridge_sink_fn_t*.

## The layouts

A host can be in any language that can call C.
Thus the header gives the size of each struct and the offset of each field, and asserts them.
The compiler adds no padding. A gap is a field `pad`, or `pad0`, `pad1` and `pad2` in a struct with more gaps.

Write zero into a gap. Bridge does not read it.
A pointer and a `size_t` have 8 bytes. The library has no build for a smaller pointer.

## BRIDGE_HOST_ABI

`BRIDGE_HOST_ABI` is the number of the host seam: the version of the structs and the functions of this header.
A host gives the number that it was built against in `abi`, the first field of `bridge_config_t`.
`bridge_create` reads `abi` before any other field, and refuses a different number.
Until the first release of Bridge, the number stays 1, and the structs can change.
After the first release, a change to a struct or to a meaning of this header increments the number.

## Settings

A setting is a key and a value of one of four kinds: a `bridge_setting_t`.
A deployment gives settings to a backend in `bridge_backend_t`, and to a module in `bridge_module_config_t`.

| Field | Content |
| --- | --- |
| `key` | The name of the setting. Not null, and once in the settings of a module. |
| `kind` | The kind of the value: an item of `enum bridge_setting_kind`. Zero is unset. |
| `pad` | 7 bytes of padding. |
| `v` | The value: a `bridge_value_t`. |

`bridge_value_t` is a union. `kind` tells which field of the union holds the value:

| `kind` | Field | Value |
| --- | --- | --- |
| `BRIDGE_BOOL` | `b` | A `bool`. |
| `BRIDGE_INT` | `i` | An `int64_t`. |
| `BRIDGE_FLOAT` | `f` | A `double`. |
| `BRIDGE_STRING` | `s` | A text that ends with a zero. |

## bridge_backend_t

`bridge_backend_t` is a backend of the deployment: a library that runs modules.

| Field | Content |
| --- | --- |
| `name` | The name of the backend. A module names the backend that runs it by this name. |
| `library` | The path or the name of the shared library of the backend. |
| `settings` | The settings of the backend, `setting_count` of them. |
| `setting_count` | How many settings `settings` holds. |
| `pad` | 4 bytes of padding. |

`bridge_create` opens the library of each backend, and starts the backend with its settings.
Bridge gives the settings to the backend unread. The backend refuses a key that it does not know.
A name with no folder goes to the search of the operating system.
On Windows, the folder of a library with a path is also searched for the libraries that it needs.
`docs/extern/bridge_backend.h.md` tells what the library of a backend holds.

## bridge_budget_t

`bridge_budget_t` states how many of each thing a world can hold at one time, and how many bytes.
`bridge_create` takes the memory of the whole budget. A phase and a barrier take no more memory.
Zero is unset, and `bridge_create` refuses it.
A record names a key as a TOML file does: `[budget] entities` is `entities` of `budget`.

| Field | Content |
| --- | --- |
| `entities` | The size of the entity index. Index 0 is no entity, thus the world holds `entities` minus 1 entities. |
| `tables` | The tables at one time. A table holds the entities that carry one set of components: an archetype. |
| `types` | The declared types at one time: the components, and the structs, enums and bitmasks that their fields name. |
| `systems` | The systems and the observers at one time. |
| `terms` | The most terms that one system or one observer can have. |
| `modules` | The modules that are loaded at one time. |
| `pad` | 4 bytes of padding. |
| `persistent` | The size of the persistent arena, in bytes. Each module takes a share of it. |
| `columns` | The size of the column region, in bytes: the rows of all the tables and sets, and the cells of the other components. |
| `sparse_sets` | How many components the world can keep in sparse sets, or `BRIDGE_NONE`. |
| `direct_indexed` | How many components can be direct-indexed, or `BRIDGE_NONE`. |

`entities`, `persistent` and `columns` are each at most `UINT32_MAX`, since an offset has 32 bits.
The shared memory is at most 4 GiB, since an offset into it has 32 bits.
A sparse set and a direct-indexed component keep something for each entity, carried or not.
Thus `bridge_create` takes their room, and `sparse_sets` and `direct_indexed` say how many there can be.

A type that two modules declare counts once. A reload does not need room for the systems of a module two times.
While a reload moves the rows of a component to a new layout, the component counts two times against `types`.

### Running out

This table tells what happens when a budget runs out:

| What runs out | Where | Result |
| --- | --- | --- |
| `entities`, or the `spawns` of a module | `spawn` | The invocation fails. |
| The `persistent` share of a module | `alloc` | The invocation fails. |
| The `changes` of a module | `attach`, `detach`, `destroy` | The invocation fails. |
| `types`, `systems`, `terms`, `sparse_sets`, `direct_indexed` | `declare`, `add_system`, `observe` | The init of the module fails. |
| `columns`, for a singleton or a direct-indexed component | `declare` | The init of the module fails. |
| `modules`, `persistent` | `bridge_load` | Bridge refuses the module. |
| The `spans` of a module | `bridge_load` | The load fails. |
| The `spans` of a module | A barrier | Bridge stops the module. |
| `tables`, `columns` | A barrier | Bridge does not apply the changes to an entity, and each module that asked one fails at that barrier. |
| The `log_ring` of a module | A line of the log | The ring drops the line, and a record counts it. |

An entity whose changes Bridge does not apply keeps its components and its values, and no observer hears of it.
Bridge places as many entities as fit, in the order of their first change.
Bridge never gives a row memory from outside `columns`.

## bridge_order_t

`bridge_order_t` is a pair of systems that conflict in one phase, in the order that they run.
A pair can also order two observers.

| Field | Content |
| --- | --- |
| `before` | The full name of the system that runs first. |
| `after` | The full name of the system that runs second. |

The full name of a system is the identity of its module without the version, a colon, and the name of the system.
For example, `game:actors:movement` is the system `movement` of the module `game:actors@1.0.0`.
A pair can name a system that is not loaded.
A pair wins over the order in which a module registered its systems.
`docs/extern/bridge_abi.h.md`, *Conflicts* tells when two systems conflict.

## bridge_config_t

`bridge_config_t` is a deployment: all that `bridge_create` makes a world of.

| Field | Content |
| --- | --- |
| `abi` | `BRIDGE_HOST_ABI`. It is the first field, and stays the first. |
| `pad2` | 4 bytes of padding. |
| `backends` | The backends, `backend_count` of them. A backend runs each module, thus a deployment has one at least. |
| `backend_count` | How many backends `backends` holds. |
| `pad0` | 4 bytes of padding. |
| `phases` | The names of the phases, `phase_count` of them. |
| `phase_count` | How many names `phases` holds. |
| `pad1` | 4 bytes of padding. |
| `order` | The pairs of systems in the order that they run, `order_count` of them. |
| `order_count` | How many pairs `order` holds. Zero is no pair. |
| `threads` | How many threads run a phase. The thread that calls `bridge_run_phase` counts. Zero is unset. |
| `budget` | The budget of the world: a `bridge_budget_t`. |

A phase is a name, and nothing more. Bridge has no phase of its own, thus a deployment declares one at least.
Each name of a phase is different, and not empty.
The host calls the phases in the order that it wants, as often as it wants.
The world keeps `threads` minus 1 threads of its own for as long as it exists.
At `threads` of 1, the thread of the host runs all the systems, in an order that never changes.

## bridge_phase_name_t

`bridge_phase_name_t` maps a name of a phase that a module uses to a phase of the deployment.
The author of a module did not see the deployment, thus a module can use other names than the deployment.

| Field | Content |
| --- | --- |
| `from` | The name that the module uses. |
| `to` | The name of a phase of the deployment. |

If no map holds a name, Bridge looks for the name, as it is, among the phases of the deployment.
A load refuses a map with an empty name, a name that two maps hold, and a phase that the deployment did not declare.

## bridge_flag

`enum bridge_flag` is the value of a key that is yes or no.
A key left at zero is unset, and a load refuses it: Bridge does not take it for no.

| Item | Value |
| --- | --- |
| `BRIDGE_UNSET` | 0: nobody stated the key. |
| `BRIDGE_TRUE` | 1: yes. |
| `BRIDGE_FALSE` | 2: no. |

## BRIDGE_NO_LIMIT and BRIDGE_NONE

Where none is a correct value, two constants state it. They are different, since they say opposite things:

| Constant | Value | Meaning |
| --- | --- | --- |
| `BRIDGE_NO_LIMIT` | -1 | A limit that is none: the module can use all that there is. |
| `BRIDGE_NONE` | -2 | A reservation that is none: the module gets nothing. |

Each key takes one of the two. A load refuses the other one, and each other number below zero.

## bridge_on_error

`enum bridge_on_error` states what the loss of a module costs the deployment. Zero is unset.

| Item | Meaning |
| --- | --- |
| `BRIDGE_FAIL` | The world stops when the module fails to load, when Bridge stops the module, and when a failed reload stops it. |
| `BRIDGE_SKIP` | The world goes on without the module. |

A config that Bridge refuses before the module exists stops nothing: `on_error` does not apply to it.
A stopped world refuses `bridge_run_phase`, and each load, unload and reload of a module other than the one that stopped it.
A load or a reload of that module that succeeds lets the world run again.
An unload of that module lets the world run again, without it.
`bridge_list_modules` and `bridge_destroy` work on a stopped world.

## bridge_module_config_t

`bridge_module_config_t` is what a module is to the deployment.
The host makes one flat struct of the values that the author of the module states and the values that the deployment states.
The host states each field: zero is unset, and a load refuses it. A count of an array can be zero.

| Field | Content |
| --- | --- |
| `identity` | The identity of the module: `namespace:name@version`, for example `tri:triangle@0.1.0`. At most 127 bytes. |
| `abi` | The number of the module seam that the module was built against: `docs/extern/bridge_abi.h.md`, *BRIDGE_ABI*. |
| `trusted` | `BRIDGE_TRUE` if the code of the module needs no containment. |
| `reloadable` | `BRIDGE_TRUE` if the author of the module states that a reload can replace it. |
| `hot_reload` | `BRIDGE_TRUE` if the deployment lets `bridge_reload` replace the module. It cannot allow what `reloadable` forbids. |
| `pinned` | `BRIDGE_TRUE` if each system and observer of the module runs on the thread that loaded it. |
| `on_error` | What the loss of the module costs: an item of `enum bridge_on_error`. |
| `pad0` | 7 bytes of padding. |
| `max_system_failures` | The count of failures of one system or observer at which Bridge stops the module, or `BRIDGE_NO_LIMIT`. |
| `work_limit` | The work that one invocation of a system can do, in the units of the backend, or `BRIDGE_NO_LIMIT`. |
| `memory_limit` | The most bytes that the own memory of the module can have, or `BRIDGE_NO_LIMIT`. |
| `init_timeout` | The longest time of one call of `init`, in nanoseconds, or `BRIDGE_NO_LIMIT`. |
| `reload_timeout` | The longest time of one call of `reload`, in nanoseconds, or `BRIDGE_NO_LIMIT`. |
| `shutdown_timeout` | The longest time of one call of `shutdown`, in nanoseconds, or `BRIDGE_NO_LIMIT`. |
| `log_level` | The highest level of a line that the module writes: 1 to `BRIDGE_LEVEL_MOST`. |
| `pad1` | 3 bytes of padding. |
| `scratch` | The size of the scratch arena of the module, in bytes. |
| `log_ring` | The size of the log ring of the module, in bytes: one record of 32 bytes at least. |
| `spans` | The size of the span region of the module, in bytes. Bridge rounds what the region must hold up to a multiple of 8. |
| `spawns` | How many entity ids the module can take between two barriers, or `BRIDGE_NONE`. |
| `persistent` | The share of the persistent arena of the module, in bytes, or `BRIDGE_NONE`. Bridge rounds the share up to a multiple of 8, and the rounded share must fit. |
| `changes` | How many bytes of changes the module can queue between two barriers, or `BRIDGE_NONE`. |
| `read` | The components of other namespaces that the module can read, `read_count` of them. |
| `read_count` | How many names `read` holds. |
| `pad2` | 4 bytes of padding. |
| `write` | The components of other namespaces that the module can write, `write_count` of them. |
| `write_count` | How many names `write` holds. |
| `pad3` | 4 bytes of padding. |
| `config` | The name of the component that the settings of the module fill, or `BRIDGE_NO_CONFIG`. Null is unset. |
| `settings` | The settings of the module, `setting_count` of them. |
| `setting_count` | How many settings `settings` holds. |
| `pad4` | 4 bytes of padding. |
| `phases` | The maps of the names of phases that the module uses, `phase_count` of them. |
| `phase_count` | How many maps `phases` holds. |
| `pad5` | 4 bytes of padding. |
| `losses` | The names of what a reload accepts to lose, `loss_count` of them. |
| `loss_count` | How many names `losses` holds, or `BRIDGE_NONE`. |

The namespace of a module is the part of its identity before the first colon.
Bridge finds a loaded module by its identity without the version.
Thus a load refuses a module whose identity without the version is loaded already.

### The limits

`max_system_failures`, `work_limit`, `memory_limit` and the three timeouts are limits.
A limit above zero, and some other keys, demand a capability of the backend:

| Key | Capability |
| --- | --- |
| `trusted` of `BRIDGE_FALSE` | `BACKEND_UNTRUSTED` |
| `work_limit` above zero | `BACKEND_WORK_LIMIT` |
| `memory_limit` above zero | `BACKEND_MEMORY_LIMIT` |
| A timeout above zero | `BACKEND_TIMEOUT` |
| `hot_reload` of `BRIDGE_TRUE` | `BACKEND_RELOAD` |

A load refuses a config that demands a capability that its backend does not have. Bridge does not give less than a config asks.
`BRIDGE_NO_LIMIT` demands nothing, thus the backend can compile the module with no count and no check in it.
`docs/extern/bridge_backend.h.md`, *backend_capability* tells what each capability promises.

Bridge sets `work_limit` again before each invocation of a system: it bounds one invocation.
A timeout bounds one call of the export of its name. `memory_limit` is a ceiling for the life of the instance.
A system that never returns, with no work limit, holds `bridge_run_phase` for ever.
A work limit stops a system at the same place on each run. A timeout can stop a call on a slow machine, and not on a fast one.

### Failures

Bridge counts each system and each observer alone against `max_system_failures`.
An invocation that fails adds 1 to the count of its system. An invocation with no failure subtracts 1, down to zero.
The module also has a count of its own.
A barrier adds 1 to it when the barrier cannot apply a change of the module, or refuses a destroy of the module.
A barrier that applies all the changes of the module subtracts 1, down to zero.

When a count gets to `max_system_failures`, Bridge stops the module at the next barrier.
A stopped module runs no system and no observer, and its components stay. A reload brings it back, and an unload removes it.

### The private memory

`scratch`, `log_ring` and `spans` size the private memory of the module: `docs/extern/bridge_abi.h.md`, *The memories*.
Together they are at most 4 GiB, since an offset into the private memory has 32 bits.
`spans` holds what each barrier writes for the module: its spans, and a cell for each component it declared in each table.

A load fails if `spans` cannot hold the slots of the module and a cell for each component in each possible table.
Bridge stops a module whose spans no longer hold what its queries match.
The author of a module states a value for `spans`, and the deployment can correct it. A reload that states more gets more.

### The reservations

`spawns`, `persistent` and `changes` reserve room in the shared memory for the module alone.
Thus what a module gets does not depend on the modules that ran beside it.

- `spawns`: Bridge sets these ids aside at each barrier. They are entities with no component, and they count against `entities`.
- `persistent`: the share must fit in one piece of what is left of the arena. A reload can state more, but not less.
- `changes`: each `attach`, `detach` and `destroy` takes 24 bytes, and an attach also takes its value, rounded up to 8 bytes.

At an unload, the share of the module goes back to the arena, unless a row that stays holds a string in it.
A string that a component keeps stays good for as long as its row stays.

### The settings of a module

The settings of a module fill one component of the module: the struct that `config` names.
The module declares that struct in its `init`, as a singleton: `docs/extern/bridge_abi.h.md`, *Storage layouts*.
After `init`, Bridge writes each setting into the field that its key names. A field that no setting names is zero.

After the load, the component is the state of the module. A system with a grant can change it.
A reload does not write the settings again. A component holds the settings of one module at a time.
A module with no settings component states `BRIDGE_NO_CONFIG`, the text `none`.
No component has that name, since the name of each component has a namespace.

A setting fills a field by the type of the field:

| Field | Setting |
| --- | --- |
| A `bool` | `BRIDGE_BOOL`. |
| An integer or a `char` | `BRIDGE_INT`, if the field holds the value. |
| A `float` or a `double` | `BRIDGE_FLOAT`, or `BRIDGE_INT`. |
| An enum | `BRIDGE_INT` that the enum holds, or `BRIDGE_STRING`: the name of a constant of the enum. |
| A bitmask | `BRIDGE_INT` that the bitmask holds. |
| An array of `char` | `BRIDGE_STRING`: the bytes and a zero after them, if they fit. |
| A `bridge_text_t` | `BRIDGE_STRING`: Bridge copies the text into the share of the module in the persistent arena. |
| A struct, or another array | No setting fills it. |

### The grants

`read` and `write` are the grants of the module: `docs/extern/bridge_abi.h.md`, *Grants* tells what each allows.
If two modules can write one component, their systems can conflict. Then the deployment orders them with `order`.

### A pinned module

`pinned` is a key of the author: a module that drives something that one thread must drive, for example a window, states it.
`bridge_load` calls `init` on the thread that calls it. The systems and observers of a pinned module run on the thread that calls `bridge_run_phase`.
While a pinned module is loaded, call each function from the thread that loaded the module.
Bridge refuses `bridge_run_phase`, `bridge_load`, `bridge_unload`, `bridge_reload` and `bridge_destroy` from another thread.
Pinned modules run one after another, since they share one thread.

### The losses

`losses` names what a reload of the module accepts to lose: `docs/extern/bridge_abi.h.md`, *When a type changes*.
A name is a component and its field, for example `game:Position.z`, or `game:Body.at.z` one level down.
For the rows of a toggle that are off, the name is the component alone, for example `game:Shield`.
A reload fails for a loss that is not in `losses`, and for a name in `losses` that it does not lose.
A load loses nothing, thus a load states `BRIDGE_NONE`.

## bridge_artifact_t

`bridge_artifact_t` is a module to load or to reload.

| Field | Content |
| --- | --- |
| `cfg` | What the module is to the deployment: a `bridge_module_config_t`. |
| `backend` | The name of the backend that runs the module: one of the backends of the deployment. |
| `bytes` | The image of the module, `len` bytes, or null. |
| `len` | How many bytes `bytes` holds. |
| `path` | The path of the module, or null. |

Bridge gives `bytes`, `len` and `path` to the backend unread.
A backend that needs the bytes or the path, and gets null, refuses the module.
Bridge does not find modules: the host finds each module, and gives it to `bridge_load`.

## bridge_record_t

`bridge_record_t` is a record: what Bridge tells the host. Bridge writes no file.
The sink of the world gets each record.

| Field | Content |
| --- | --- |
| `identity` | The identity of the module that the record comes from or is about, or null. |
| `system` | The name of the system or the observer that the record comes from, or null. |
| `text` | The text, `len` bytes. A zero does not always follow it. |
| `len` | How many bytes `text` has. |
| `barriers` | How many barriers the world had made when the record was made. |
| `place` | The place of the system in its phase, from 0. Zero in a record of Bridge. |
| `arg` | Two numbers that a module writes with a line: a value, and what it is set beside. Zero in a record of Bridge. |
| `level` | The severity of the record: 0 to 7. Lower is worse. |
| `pad` | 3 bytes of padding. |

A record of Bridge that names a module in its text can have a null `identity`.
A line of the log of a module has the `barriers` and the `place` that the module wrote.

`enum bridge_level` names four severities of syslog, RFC 5424, by their numbers. 0 is the worst, and 7 is the most detail.

| Item | Value |
| --- | --- |
| `BRIDGE_ERR` | 3: an error. |
| `BRIDGE_WARN` | 4: a warning. |
| `BRIDGE_INFO` | 6: information. |
| `BRIDGE_DEBUG` | 7: detail. |

`BRIDGE_LEVEL_MOST` is 7: the highest level.
A module can write each of the eight levels: `docs/extern/bridge_abi.h.md`, *The log*.

### bridge_sink_fn_t

`bridge_sink_fn_t` is the sink: the function of the host that gets each record.
`ud` is the pointer that the host gave to `bridge_create`, and `r` is the record.

- Bridge calls the sink one time for each record.
- Bridge calls the sink only on the thread that called a function of the world, thus the sink of a world needs no lock.
- A record and its text are good only during the call. To keep a line, copy it.
- Bridge gives the lines of the logs to the sink at each barrier, after the systems of the phase returned.
- A module whose `init` or `reload` fails gives its lines too, before the record of the failure.
- The sink can call `bridge_list_modules`. An identity that it gives there is good only during that call of the sink.
- If the sink calls another function of the world, the function does nothing, gives 1, and writes no record.

A function gives its result. A record tells why. `bridge_list_modules` tells what runs.

## bridge_module_info_t

`bridge_module_info_t` is a loaded module, as `bridge_list_modules` tells it.

| Field | Content |
| --- | --- |
| `identity` | The identity of the module. It stays good until the next load, unload or reload. |
| `state` | Whether the module runs: an item of `enum bridge_state`. |
| `pad` | 7 bytes of padding. |

| Item | Meaning |
| --- | --- |
| `BRIDGE_RUNNING` | The module runs. |
| `BRIDGE_STOPPED` | Bridge stopped the module. Its systems and observers do not run, and its components stay. |

Bridge stops a module at its failure budget, a module whose spans no longer fit, and a module whose reload failed with no old instance.

## bridge_create

`bridge_create` makes a world of a deployment. It loads no module.

| Parameter | Content |
| --- | --- |
| `cfg` | The deployment. |
| `sink` | The function that gets each record. |
| `ud` | A pointer that Bridge gives to `sink` with each record. |

1. Bridge reads `abi` of `cfg`.
2. Bridge opens the library of each backend, finds its entries, and starts the backend with its settings.
3. Bridge takes the phases and the pairs of `order`.
4. Bridge starts the threads of the world.
5. Bridge makes the shared memory and the storage of the budget.
6. Each backend adopts the shared memory.

The result is the world, or nullptr if the deployment cannot start.
If `sink` or `cfg` is null, the result is nullptr, with no record.
Else the sink gets a record that tells why: *The deployment*.

## bridge_destroy

`bridge_destroy` destroys a world.

| Parameter | Content |
| --- | --- |
| `b` | The world, or null. |

1. Bridge calls `shutdown` of each loaded module, in the order that the modules loaded, each under its `shutdown_timeout`.
2. A `shutdown` that fails is a record, and the destruction continues.
3. Bridge destroys the modules, the storage, the threads and the backends.
4. Bridge closes the library of a backend that states `BACKEND_UNLOAD`. Another library stays, and the next world uses it.

The result is 0 when the world is gone, and for a null `b`.
The result is 1 if a pinned module is loaded and the call comes from another thread, or if the sink calls it.
Then Bridge destroys nothing, and the world stays whole. Call again from the thread that loaded the module, after the sink returns.
A host that ignores the result can lose a world that stays.

## bridge_load

`bridge_load` loads a module onto a world.

| Parameter | Content |
| --- | --- |
| `b` | The world. |
| `a` | The module. |

1. Bridge checks the config of the module against the deployment and against the backend.
2. Bridge makes a record of the module, takes its share of the arena, and sets ids aside.
3. The backend compiles the image, and makes an instance with the memories that the module imports.
4. Bridge calls `init` under `init_timeout`. `init` declares the types, and registers the systems and the observers.
5. Bridge writes the settings into the component that `config` names.
6. Bridge finds the export of each system and observer, and `shutdown` and `reload`.
7. The load fails if a system of the module conflicts with a system of another module, and nothing orders them.
8. A barrier applies what `init` queued, and writes the spans of the module.

The result is 0 if the module is loaded, its `init` gave 0, and the world can run it. Else the result is 1, with a record.
From the sink, the result is 1, with no record.
A refusal of the config stops nothing. A failure after the module exists stops the world, if `on_error` is `BRIDGE_FAIL`.
A load that fails leaves the storage as it was.

The module comes after the modules that loaded before it. A barrier applies the changes of the modules in this order.
When two modules conflict, the order of the loads decides which load fails. The record names the two systems.

## bridge_unload

`bridge_unload` removes a module from a world.

| Parameter | Content |
| --- | --- |
| `b` | The world. |
| `identity` | The identity of a loaded module, with its version. |

1. The systems and the observers of the module leave the schedule.
2. Bridge calls `shutdown` under `shutdown_timeout`, and applies what the module queued.
3. Each component that no other loaded module declares goes, by its `on_unload`.
4. Bridge releases the ids that it held ready for the module.
5. The share of the module goes back to the arena, unless a row that stays holds a string in it.
6. The backend destroys the instance and the image, and a barrier follows.

The result is 0 if the module was loaded. Else the result is 1, with a record.
From the sink, the result is 1, with no record.
A component that another loaded module declares stays, with its rows.
`docs/extern/bridge_abi.h.md`, *bridge_on_unload* tells what happens to the entities of a component that goes.

## bridge_reload

`bridge_reload` replaces the instance of a loaded module, and keeps its storage.

| Parameter | Content |
| --- | --- |
| `b` | The world. |
| `a` | The new module. Its identity without the version is the identity of a loaded module. |

1. Bridge checks the config as a load does. The loaded module must allow `hot_reload`, and stay on its backend.
2. The share of the arena of the module grows to the new `persistent`.
3. The systems of the old instance stand aside.
4. Without `BACKEND_SIDE_BY_SIDE`, Bridge shuts the old instance down first.
5. The backend makes a new image, a new instance and a new private memory.
6. Bridge calls `reload` of the new instance under `reload_timeout`. It declares and registers as `init` did.
7. Bridge checks the schedule, and moves the rows of each component whose layout changed.
8. Bridge shuts the old instance down, and the new instance takes its place.

The old and the new instance take entity ids and bytes of the persistent arena from one supply.
Each id and each byte goes to one instance only.

The result is 0 if the new instance runs. Else the result is 1, with a record.
From the sink, the result is 1, with no record.

A reload keeps the components, their rows and the entities. Bridge does not write the settings again.
A component that the new instance does not declare goes, as at an unload.
The new instance runs under the new config, but `persistent` can grow and not shrink.
If the new config states fewer `spawns`, the module keeps that many of the ids that it held ready. Bridge takes the others back at the next barrier.

With `BACKEND_SIDE_BY_SIDE`, a reload that fails keeps the old instance running, as it was.
Without it, a reload that fails stops the module, and keeps what it declared. A reload that succeeds brings it back.

## bridge_list_modules

`bridge_list_modules` tells which modules are loaded, and whether each one runs.

| Parameter | Content |
| --- | --- |
| `b` | The world. |
| `out` | An array of `max` items, or null. |
| `max` | How many items `out` holds. |

The result is the number of loaded modules. Bridge fills at most `max` items, in the order that the modules loaded.
A host gets the state of a module only from this function, since the log can drop a line.
A module that Bridge stopped long after its load shows as `BRIDGE_STOPPED`.

## bridge_run_phase

`bridge_run_phase` runs one phase and the barrier after it.

| Parameter | Content |
| --- | --- |
| `b` | The world. |
| `name` | The name of a phase of the deployment. |

1. The systems of the phase run. Systems of different modules run at the same time, if they do not conflict.
2. At the barrier, Bridge applies the queued changes in the order of the calls, and writes the spans.
3. The observers of what changed run. What they queue applies at the next barrier.
4. Bridge counts the failures, and gives the lines of the logs to the sink.

The systems of one module run one after another, and each one from its start to its return.
The result is 0 if the phase ran, whatever its systems did: a system that failed is a record.
The result is 1, with a record, if Bridge refuses the call: *A call refused*.
From the sink, the result is 1, with no record.

A system sees the same state of the world for the whole of a phase. An entity that a phase makes is visible in the next phase.
A host can call a phase more often than another, for example a phase of physics at a fixed rate.
Bridge does not know if a host forgets a phase.

## Messages

Bridge gives each message to the sink in a record.
In a quote, a word in angle brackets is a part that changes. `<module>` is the identity of a module.
`<reason>` is one of the texts that follow the quote, or a text that a backend wrote.
Each record of Bridge has the level `BRIDGE_ERR`, unless its group tells another level.

### The deployment

`bridge_create` writes one of these records when it refuses the deployment, and gives nullptr:

- `abi is unset`
- `the host is of host ABI <number>, and this is <number>`
- `threads is unset`
- `the deployment configures no backend, and a module is run by one`
- `backend <index> has no name`
- `backend <index> has no library`
- `backend '<backend>' is named twice`
- `backend '<backend>': <reason>`
- `cannot open backend library '<path>'`
- `backend '<backend>' would not start: <reason>`
- `backend '<backend>' cannot adopt the shared memory: <reason>`
- `the deployment declares no phase, and there is none of Bridge's to run`
- `phase <index> has no name`
- `phase '<phase>' is named twice`
- `[order] pair <index> does not name two systems`
- `[order] contradicts itself: '<system>' would run before itself`
- `[budget] <key> is unset`
- `[budget] <key> of <number> is more than an offset of 32 bits can reach`
- `[budget] types of <number>, rounded up to a multiple of 64, is more than a count of 32 bits holds`
- `[budget] tables, sparse_sets and types come to <number>, which is more than a count of 32 bits holds`
- `[budget] <key> is <number>, which is neither a count nor none`
- `[budget] cannot be satisfied: sparse_sets of <number>, with four bytes for each of <number> entities, is more than an offset of 32 bits can reach`
- `[budget] cannot be satisfied: direct_indexed of <number>, with a bit for each of <number> entities, is more than an offset of 32 bits can reach`
- `[budget] cannot be satisfied: the shared memory would be <bytes> bytes and an offset into it is 32 bits`
- `[budget] cannot be satisfied: the operating system would not give <bytes> bytes, in a reservation of <bytes> bytes`
- `[budget] cannot be satisfied: the operating system would not give storage <bytes> bytes of its own for the columns it keeps`

The reason of `backend '<backend>': <reason>` is `cannot open backend library '<path>'`, or a reason of `docs/extern/bridge_backend.h.md`, *Messages*.
The reason of `would not start` and of `cannot adopt` is the text of the backend.

### A call refused

A function writes one of these records when it refuses the call, and gives 1:

- `the world has stopped: <reason>`
- `refused: module '<module>' is pinned to the thread that loaded it, and this call comes from another`
- `phase '<phase>' is not one the deployment declared`
- `a module with no identity is refused`
- `a module with an identity of <count> bytes is refused: an identity has at most 127 bytes`
- `no module '<module>' is loaded, so none is unloaded`
- `module '<module>' is not reloaded: nothing of that name is loaded`

The reason of `the world has stopped` is in *A stopped world*.

### A module refused

`bridge_load` and `bridge_reload` write this record when the config of a module does not fit, before the module exists:

- `module '<module>' is refused: <reason>`

The reason is one of these:

- `no backend named '<backend>'`
- `it wants ABI <number> and this is ABI <number>`
- `<key> is unset`
- `<key> is <number>, which is neither BRIDGE_TRUE nor BRIDGE_FALSE`
- `<key> is <number>, which is neither a limit nor BRIDGE_NO_LIMIT`
- `<key> is <number>, which is neither an amount nor BRIDGE_NONE`
- `on_error is unset`
- `config is unset`
- `it states <count> losses, and losses is unset`
- `phase name <index> of its config is unset`
- `its config maps phase '<name>' twice`
- `its config maps phase '<name>' to '<phase>', which the deployment did not declare`
- `its log_level of <number> is not one it can be, which is 1 to <number>`
- `its changes of <bytes> bytes is more than an offset of 32 bits can reach`
- `its spawns of <count> is more than [budget] entities of <count>`
- `its scratch of <bytes>, log_ring of <bytes> and spans of <bytes> bytes come to more than an offset of 32 bits can reach`
- `a log_ring of <bytes> bytes holds no record, and one is <bytes>`
- `it is not trusted, and backend '<backend>' runs only modules that are`
- `it states <key>, which backend '<backend>' cannot keep`
- `hot_reload is stated, and its author states it is not reloadable`
- `'<module>' is loaded already`
- `[budget] modules of <count> is reached`
- `its persistent of <bytes> bytes does not fit in the <bytes> left of [budget] persistent`

### A load that failed

`bridge_load` writes this record when the module fails after it exists:

- `module '<module>' did not load: <reason>`

The reason is a text of the backend, a refusal of an entry of the module, or one of these:

- `the operating system would not give its private memory of <bytes> bytes`
- `out of memory: there was none to call its <export> with`
- `its <export> answered <status>`
- `its <export> <reason>`
- `its spawns of <count> do not fit under [budget] entities of <count>`
- `it accepts the loss of '<name>', and loses nothing of that name`
- `its spans of <bytes> bytes do not hold a column_base of <count> components by <count> tables, which with its queries is <bytes> bytes`
- `'<system>' and '<system>' both touch '<component>' in phase '<phase>', one of them writing it, and nothing orders them`
- `'<observer>' and '<observer>' both attach or detach '<component>' at the barrier, and nothing orders them`

`its <export> <reason>` tells that the export did not return: the reason is the text of the backend.
A conflict names the full names of the two systems. A conflict of observers is at the barrier.
`docs/extern/bridge_abi.h.md`, *Messages* gives the refusals of the entries.

### The settings

These reasons of `did not load` tell that Bridge cannot write the settings of the module:

- `it is handed <count> settings, and states that it has no config component`
- `its config is '<component>', which is already the settings of module '<module>'`
- `its config is '<component>', which is not a struct its init declared`
- `its config is '<component>', which is not declared a singleton`
- `its setting <index> has no key`
- `its setting '<key>' has the kind <number>, which is no kind of a setting`
- `its setting '<key>' is text, and its text is null`
- `its setting '<key>' is handed more than once`
- `out of memory: there was none for the row of its settings`
- `its settings have a key '<key>', and '<component>' has no field of that name`
- `its setting '<key>' is <kind>, and the field it names is <kind>`
- `its setting '<key>' is <number>, which the field it names cannot hold`
- `its setting '<key>' is <number>, which the enum of the field it names cannot hold`
- `its setting '<key>' is <number>, which the bitmask of the field it names cannot hold`
- `its setting '<key>' is '<text>', which the enum of the field it names has no constant for`
- `its setting '<key>' is <bytes> bytes of text, and the field it names holds <bytes> with the zero that ends it`
- `its settings hold <bytes> bytes of text, and its persistent share has <bytes> left`

### A reload

`bridge_reload` writes this record when it refuses the reload, and the module runs on as it was:

- `module '<module>' is not reloaded: <reason>`

The reason is one of these:

- `its hot_reload is false`
- `it runs on backend '<backend>' and a reload does not move it to '<backend>'`
- `its persistent is <bytes> and a reload states less, which a loaded module's share cannot become`
- `its persistent of <bytes> bytes does not fit in the <bytes> left of [budget] persistent`

`bridge_reload` writes these records when the new instance fails:

- `module '<module>' was not reloaded: <reason>`
- `module '<module>' is stopped: its reload failed after its old instance had been shut down. What it declared is kept`

The reason is a reason of `did not load`, or one of these:

- `what it keeps another way, <count> of its components, could not be moved: [budget] tables of <count> or [budget] columns of <bytes> bytes has no room for the rows where they are to go, while they are still where they were`
- `out of memory: there was none to carry across the values of what it keeps another way, <count> of its components`

`bridge_reload` writes this record, at the level `BRIDGE_WARN`, for each loss that the reload made:

- `module '<module>' is reloaded, and '<name>' is lost from <count> rows, as it accepted`

### An unload

`bridge_unload`, `bridge_reload` and `bridge_destroy` write this record when `shutdown` fails:

- `module '<module>': <reason>; its teardown continues`

`bridge_unload` writes these records, at the level `BRIDGE_WARN`:

- `module '<module>' is unloaded and its share of the persistent arena, <bytes> bytes, stays: '<component>' holds a string in it, in this many rows: <count>`
- `'<component>' is stripped, and <count> rows stay in a second table of their archetype: [budget] columns of <bytes> bytes has no room to join them to the first`

A share that stays keeps its room, thus Bridge can refuse a later load for want of room.
A share that stays goes back at a later unload, once no row that stays holds a string in it.
Rows in a second table make a second span of the same archetype.

### A phase

`bridge_run_phase` and each other function that runs a barrier write these records:

- `system '<system>' failed: <reason>`
- `observer '<observer>' failed: <reason>`
- `it answered <status>`
- `module '<module>' is stopped: its system '<system>' reached its max_system_failures of <count>`
- `module '<module>' is stopped: it reached its max_system_failures of <count>, by entities it changed that could not be placed`
- `module '<module>' is stopped: it reached its max_system_failures of <count>, by destroys of entities that carry nothing it may write`
- `module '<module>' is stopped: what its queries match no longer fits in its spans of <bytes> bytes`
- `module '<module>' is stopped: the list of its observer '<observer>' no longer fits in its spans of <bytes> bytes`
- `module '<module>': [budget] tables of <count> is used up. Not applied, of the changes it asked: <count>`
- `module '<module>': [budget] columns of <bytes> bytes is used up. Not applied, of the changes it asked: <count>`

The reason of `failed` is `it answered <status>`, the text of the backend, or a refusal of an entry.
For an observer, a record that names a system names the observer.

This record, at the level `BRIDGE_WARN`, counts the lines that a full log ring dropped:

- `<count> log lines were dropped: its ring holds <count>`

### A stopped world

Bridge writes one of these records when a module of `BRIDGE_FAIL` stops the world:

- `the world has stopped: module '<module>' failed and its on_error is fail`
- `the world has stopped: module '<module>' was stopped and its on_error is fail`
- `the world has stopped: module '<module>' could not be reloaded and its on_error is fail`

Each later call that the stopped world refuses writes the same record.
Bridge writes one of these records, at the level `BRIDGE_INFO`, when the world runs again:

- `the world runs again: module '<module>' is back`
- `the world runs again: module '<module>' is unloaded, and it goes on without it`

### Out of memory

A function that cannot get memory writes this record, and fails as for any other refusal:

- `out of memory: there was none for <what>`

`<what>` names what Bridge needed the memory for.
Bridge takes memory only in the functions that make or destroy a world, and that load, reload or unload a module.
It never takes memory in a phase or at a barrier. A function that fails for want of memory leaves the world as it was.
