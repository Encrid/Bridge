# src/lib/hash.h

This header gives `hash_fnv1a`, the hash of a run of bytes.
The abi makes the fingerprint of a type with it, and the checksum pass hashes a column with it.
See `docs/intern/lib/abi/abi_stated.c.md`, *The fingerprint*, and
`docs/intern/lib/storage/storage_validate.c.md`.

## Functions

| Function | Result |
| --- | --- |
| `hash_fnv1a` | FNV-1a over `len` bytes, 64 bits. Equal bytes hash equal on every platform. |
