# src/bridge_phases.c

This file takes the phases of a deployment. A phase is a name, and nothing more.

## No phase names

Bridge ships no phase name, and no default set of names.
A phase name is a guess about the shape of a frame, and two kinds of program do not want the same phases.
So each deployment declares its own phases, and a deployment that declares none is refused: Bridge has no phase of its own to run.
A phase has no order, because the host calls the phases in the order that it chooses.
A phase has no other property either, and a phase that the host never calls does not run.

## The steps

1. Refuse a deployment with no phase.
2. For each phase, refuse an empty name, or the name of a phase before it.
3. Copy the name. Count the phase only then, so that a world that is refused frees it.

Each lookup of a phase by its name uses `bridge_find_phase`.
It finds the phases of the deployment, and the names that a module maps.

## Functions

| Function | Result |
| --- | --- |
| `add_phase` | Takes the phase `i` of the deployment. 1, with a record, if it has no name, has the name of another, or there is no memory for its name. |
| `bridge_name_phases` | Takes the phases of the deployment, each a name. 1, with a record, if there is none or one is refused. |
| `bridge_find_phase` | Gives the index of the phase name of `len` bytes among `count` names, or `count`. |
