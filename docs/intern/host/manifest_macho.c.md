# src/host/manifest_macho.c

This file locates the `.bridge` section of a Mach-O library.

`manifest_find_macho` walks the load commands and inspects each 64-bit segment command.
`in_segment` scans the sections of a segment for the name `.bridge` under the `__TEXT` segment.
Once it has the section, the finder reads the size and offset and verifies the bounds.

## Functions

| Function | Result |
| --- | --- |
| `manifest_find_macho` | Reports the place and size for Mach-O, or gives false. |
