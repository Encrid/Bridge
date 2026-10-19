# src/host/manifest_find.c

This file picks the file format and finds the manifest section of a PE library.

## The formats

`FORMATS` lists the magic head, its length, and the finder for each library format.
`manifest_find` first tests for a wasm module, then matches a head against the list.
It gives `wasm` or `native` as the backend, and clears the size when no section is present.
`find_library` reads the PE section table and takes the `.bridge` section, capped by its raw size.
`manifest_number` reads a little-endian integer and stays within the bytes.

## Functions

| Function | Result |
| --- | --- |
| `manifest_number` | Reads a width of bytes as a number, or 0 when out of range. |
| `manifest_find` | Gives the backend name, and sets the section place and size. |
