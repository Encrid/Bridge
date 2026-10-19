# src/host/file_bytes.c

This file loads a whole file into memory and hashes a block of bytes.

`measure_file` seeks to the end to find the size, then seeks back to the start.
`read_whole` opens the file, allocates the size plus one byte, and reads it in a single call.
`hash_bytes` runs the 64-bit FNV-1a hash over the bytes.

## Functions

| Function | Result |
| --- | --- |
| `read_whole` | Gives the bytes of a file and sets the length, or null. |
| `hash_bytes` | Gives the FNV-1a mark of a block. |
