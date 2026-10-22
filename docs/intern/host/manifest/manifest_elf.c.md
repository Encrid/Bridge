# src/host/manifest/manifest_elf.c

This file finds the `.bridge` section of an ELF library.

`manifest_find_elf` reads the section header table and its count from the ELF header.
It reads the section-name string table, then scans each header for the name `.bridge`.
`named` compares a name at an offset without reading past the end.
The finder returns the place and size of that section when the bounds hold.

## Functions

| Function | Result |
| --- | --- |
| `manifest_find_elf` | Reports the place and size of the ELF `.bridge` section, or gives false. |
