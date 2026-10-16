# src/config.c

This file writes the settings of a module into the component that its config names.

## The settings component

The settings that a host hands for a module are the initial value of a singleton component.
The module declares the struct in its schema header like any other, and declares it a singleton.
Bridge gives the component its value at `init`, by matching the keys of the settings to the names of its fields.
A key that the struct has no field for is refused at `init`, by its name: a typo is not ignored.

The author names the component with the key `config` of the manifest, because only the author knows which struct holds the settings.
A module with no settings component states `BRIDGE_NO_CONFIG`, which is `none`.
That names no component, because the name of each component has a namespace and a colon.
A `config` left null is unset and refused: an author who forgot it is told, and is not taken to mean none.
Settings handed to a module that states `none` are refused.
A `config` that `init` did not declare as a struct is refused by its name.

The component that `config` names must be a singleton, or the load is refused by its name.
It is one value on no entity, so there cannot be a second value.
A row on an entity cannot promise that, because a module that declared the component could attach it to another entity.
The module reads the value where it is, through its header, and the value goes by its `on_unload` when the module goes.
A component is the settings of one loaded module at a time.

After `init`, the component is the live state.
A change of a setting while the deployment runs is a write of the component, by a module that has the grant.
It is not a call and not a reload: the config gives the initial value, and the component holds the running state.
The settings that are Bridge's own, for example a work limit or a log level, change by a reload.
To keep a changed setting across a restart, a native module writes a file, and the host reads it back. Bridge never sees it.

## The steps

`config_seed` runs two times for a load, with the same steps.
First it checks, and writes nothing that outlives the load: `docs/intern/module_start.c.md`.
Then, after the schedule accepts the load, it writes: `docs/intern/bridge_load.c.md`.
A load that fails between the two leaves storage and the arena as they were.

The steps:

1. If `config` is `BRIDGE_NO_CONFIG`, refuse any setting, and stop.
2. Find the component among the types that `init` declared.
3. Refuse it if it is not a struct, not a singleton, or already the settings of another module.
4. Refuse a setting with no key, a text setting with a null text, and a key that comes twice: `src/config_field.c`.
5. Find where the text of the settings goes: `docs/intern/config_room.c.md`. Only the second run moves the cursor of the share.
6. Write each setting into one value, in order, and stop at the first refusal.
7. If no setting is refused, the second run gives the singleton that value. It records the component as the settings of the module.

The reason of a refusal goes into the error of the load.
A reload does not write the settings again: `docs/intern/module_start.c.md`.

## Functions

| Function | Result |
| --- | --- |
| `check_none` | Refuses settings handed to a module that states it has no config component. |
| `check_taken` | Refuses a config component that is already the settings of another module: it is one value, so it is one module's. |
| `find_config` | Finds the component that the config of the module names, among what its `init` declared. It must be a struct and a singleton that is no other module's settings. |
| `seed_all` | Writes each setting into one value of the config component, and gives the singleton that value when no setting was refused. Refuses, with a reason, if there is no memory for the value. |
| `config_seed` | Checks the settings of a module, or, with `writing`, writes them into the component that its config names. Gives 0, or 1 with `err`. |
