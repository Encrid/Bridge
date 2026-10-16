# src/module_check.c

This file holds the checks that a load and a reload both make from a config alone, before the artifact is touched.
It also finds a loaded module by its identity.

## The identity

The identity of a module is `namespace:name@version`, with the ABI version beside it.
It has four uses: the key of its config, the key of its grants, the namespace of its components, and the name of its systems.
`[order]` names a system by the identity without its version, which is the stem.
Bridge gets the identity as data, in the config of the artifact, so a host that cannot supply an identity cannot load a module.

The identity comes from the manifest of the module: a named section of the artifact that the host reads.
The manifest holds the identity, then the values that the author states, as `key=value` texts that end with a zero byte.
It holds what only the author knows, for example the ABI version, `pinned` and the size of `scratch`.
The host combines these values with those of the deployment, and Bridge sees one flat struct.

A module is refused if it has no identity, if it names no backend of the deployment, or if it was built for another ABI.

## The stem

`module_find` compares stems, not identities.
A system is named by its module without the version, so the stem is what tells two modules apart.
Thus two modules that differ only in their version cannot be loaded together: nothing could tell their systems apart.

## The order of the checks

`module_check` makes the checks in this order, and stops at the first refusal:

1. The identity, the backend and the ABI.
2. The flags: `module_check_flags`.
3. The phase names: `module_check_phases`.
4. The keys that somebody must state: `module_check_stated`.
5. The amounts that must fit: `module_check_fit`.
6. What the backend can keep: `module_check_backend`.

Then it finds what the config demands of the backend:

| Demand | When |
| --- | --- |
| `BACKEND_UNTRUSTED` | `trusted` is not `BRIDGE_TRUE`. |
| `BACKEND_WORK_LIMIT` | `work_limit` is above 0. |
| `BACKEND_MEMORY_LIMIT` | `memory_limit` is above 0. |
| `BACKEND_TIMEOUT` | `init_timeout`, `reload_timeout` or `shutdown_timeout` is above 0. |

The demands go to the backend when it compiles the module: `docs/intern/module_instance.c.md`.

## Room for a load

`module_check_room` refuses a load in three cases:

- A module of the same stem is loaded already.
- The world has `[budget] modules` modules already.
- The `persistent` share of the module, rounded up to a multiple of 8, is longer than the longest piece left in the persistent arena.

## Functions

| Function | Result |
| --- | --- |
| `module_find` | Gives the loaded module of an identity, whatever its version, or `nullptr`. |
| `find_backend` | Gives the opened backend of a name, or `nullptr`. A null name finds none. |
| `check_whose` | Refuses an artifact with no identity, one that names no backend there is, and one built for another ABI. |
| `module_check_room` | Refuses a module that is loaded already, one more than `[budget] modules`, or a share that does not fit. Gives 1 with a record. |
| `module_check` | Makes every check that a load and a reload share, and finds the demands of the config. Gives 1 with a record. |
