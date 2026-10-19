# src/host/host_meet.c

This file meets a file: it reads the manifest and either records a new module or replaces a loaded one.

## The meeting

`find_file` gives the slot of a path, or a free slot.
`meet` reads the manifest; a loaded file goes to `replace`, and a new file keeps its path and bytes.
`replace` turns down a module whose ABI differs or that is not reloadable, then calls `bridge_reload`.
`keep_old` decides whether to hold the old module. A bad manifest prints `not a module` or `not loaded`.

## Functions

| Function | Result |
| --- | --- |
| `find_file` | Gives the slot of a path, or a free slot, or null. |
| `meet` | Records a new module and gives true, or replaces a loaded one and gives false. |
