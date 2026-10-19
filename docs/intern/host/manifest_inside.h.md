# src/host/manifest_inside.h

This private header is shared by the sources that read a manifest.
It defines `format_t`, which pairs a magic head with the function that locates the section.
It also declares the helpers for numbers, the per-format finders, and the key taker.

## Functions

| Function | Result |
| --- | --- |
| `manifest_number` | Reads a little-endian number of a given width, with a bounds check. |
| `manifest_find_elf`, `manifest_find_macho`, `manifest_find_wasm` | Find the manifest section of one format. |
| `manifest_find` | Picks the format from the head and gives the backend name. |
| `manifest_take` | Takes one `key=value` word into `manifest_t`. |
