# src/schedule/schedule_validate.c

This file names the system that wrote a column it did not declare to write.
It runs after a system when `validate` of the world is on:
`docs/intern/schedule/schedule_dispatch.c.md`, *The check*.

## Why a system runs alone

A changed column is the doing of one system only when one system ran.
So `validate` makes the schedule run the systems one at a time:
`docs/intern/schedule/schedule_run.c.md`, *A level*.
It looks for a write that strayed, a write through a bad pointer. A native module can make one; the sandbox of a wasm module cannot.
Such a write is the same on one thread or on many, so running alone hides nothing it looks for.
A fault that only a race brings is left to other tools.

## The check

`schedule_validate_system` asks storage which columns the system changed:
`docs/intern/storage/storage_validate.c.md`.
For each, `may_write` looks among the terms of the system for a write term of that type.
A system with such a term is right, and nothing is said.
A system with none wrote a column it may not, and a record names the system and its module.
The record is a warning, so it reaches the host without counting as a failure of the world.

## Functions

| Function | Result |
| --- | --- |
| `may_write` | Whether a write term of the system names the type of the changed column. |
| `on_changed` | Writes a warning that names the system, unless a write term of the system names the type. |
| `schedule_validate_system` | Asks storage which columns the system changed, and judges each against the terms of the system. |
