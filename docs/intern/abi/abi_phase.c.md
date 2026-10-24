# src/abi/abi_phase.c

This file reads the phase that a system names when the module registers it.

## The phase of a system

A system names its phase in the words of its module: `docs/extern/bridge_abi.h.md`, *add_system*.
Bridge looks for it first among the names that the deployment maps for this module: `docs/intern/module/module_check_phases.c.md`, *The phase names of a module*.
Then it looks for the name as it is written among the phases of the deployment: `bridge_find_phase`.
If it finds the name in neither place, Bridge refuses the system: `docs/extern/bridge_abi.h.md`, *add_system and observe*.
A reason cuts the name of a phase to 40 characters, as the reasons of the schedule do.

## A name used as it is

A module can name a phase of the deployment as it is.
At the same time, one of its maps can put another name onto that phase.
Then two names of the module are one phase, and a barrier no longer separates their systems.
So Bridge refuses the registration, and the reason names the other name: `docs/intern/module/module_check_phases.c.md`, *One module, different phases*.

## Functions

| Function | Result |
| --- | --- |
| `read_as_it_is` | Finds a phase of the deployment by its name, and refuses it if a map of the module puts another name onto it. 1 with a refusal. |
| `abi_read_phase` | Finds the phase that a system names: through the maps of the module, or as it is. 1 with a refusal. |
