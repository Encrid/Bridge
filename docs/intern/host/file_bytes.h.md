# src/host/file_bytes.h

This small header declares the two helpers that load and hash a file.

## Functions

| Function | Result |
| --- | --- |
| `read_whole` | Reads a whole file into fresh memory and reports its length. |
| `hash_bytes` | Gives the FNV-1a hash of a block of bytes. |
