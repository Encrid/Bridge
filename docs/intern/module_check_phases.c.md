# src/module_check_phases.c

This file refuses the phase names of the config of a module that cannot be mapped.

## The phase names of a module

A module names its phases in its own words, and the deployment says which of its phases each name is.
The author of a module never saw the deployment, and Bridge ships no phase names, so two authors can use two names for one phase.
Why Bridge ships no phase names is in `docs/intern/bridge_phases.c.md`, *No phase names*.
So the config of a module carries a table of maps, `phases`.
Each map goes from a name that the module uses to a phase that the deployment declared:

```toml
[module."mods:physics".phases]
tick = "update"
```

A name that no map holds is looked for among the phases of the deployment, as it is written.
So a module that already uses the name of the deployment needs no map.
The deployment resolves the names because it is the only party that has seen both sides.

## One module, different phases

The maps translate names. They never put two phases of one module into one phase of the deployment.
A barrier follows each phase, so it stands between the systems of two phases of one module: `docs/extern/bridge.h.md`, *bridge_phase_name_t*.
In one phase, those systems could conflict in a way that the author never saw, and the author cannot order them: `docs/extern/bridge_abi.h.md`, *Conflicts*.
Thus Bridge refuses the load for two maps onto one phase.
`docs/intern/abi_phase.c.md` refuses the other way to merge two phases: a name used as it is, onto which a map puts another name.

## The refusals

Bridge refuses the load for a map with no name on one side, and names the place of the map in the table.
It refuses a name mapped twice and two names mapped to one phase, and names them.
It also refuses a map to a phase that is not a phase of the deployment, and names it: `docs/extern/bridge.h.md`, *bridge_phase_name_t*.

## Functions

| Function | Result |
| --- | --- |
| `find_from` | Gives the first map before map `i` whose name is `from`, or `i`. |
| `find_to` | Gives the first map before map `i` whose phase is `to`, or `i`. |
| `check_phase` | Refuses map `i` of a config if it is unset or maps its name twice. Also refuses it onto the phase of an earlier map, or onto a phase that `bridge_find_phase` does not find. |
| `module_check_phases` | Refuses a config whose phase names cannot be mapped. |
