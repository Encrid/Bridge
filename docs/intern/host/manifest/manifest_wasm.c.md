# src/host/manifest/manifest_wasm.c

This file finds the manifest of a wasm module.

`manifest_find_wasm` steps through the sections after the eight-byte head.
At each custom section it reads the name and weighs it against `bridge:manifest`.
`read_leb` reads an unsigned LEB128 length. `is_manifest` matches the section name.
The finder sets the place and size of the manifest body when the name agrees.

## Functions

| Function | Result |
| --- | --- |
| `manifest_find_wasm` | Reports the place and size of the manifest, or gives false. |
