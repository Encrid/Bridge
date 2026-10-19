# src/host/manifest.h

This header defines `manifest_t` and declares the two readers of a manifest.
A `manifest_t` holds the identity of a module, its backend, and the author keys `config`, `settings`, `abi`, `pinned` and `reloadable`.
The flags `has_pinned` and `has_reloadable` record whether the manifest stated each one.

## Functions

| Function | Result |
| --- | --- |
| `manifest_read` | Fills a `manifest_t` from the bytes of a file. Gives a reason on refusal, else null. |
| `manifest_get_value` | Points at the raw value of one key in the manifest, and reports its size. |
