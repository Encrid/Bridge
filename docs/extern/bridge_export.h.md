# include/bridge_export.h

`include/bridge_export.h` gives `BRIDGE_API`, the mark of each function that the libraries of Bridge export.
`include/bridge.h` and `include/bridge_backend.h` include it, so 1 macro marks the functions of the 2 headers.

## BRIDGE_API

`BRIDGE_API` comes before each function that the shared library exports:

| Build | `BRIDGE_API` |
| --- | --- |
| The library, on Windows | `[[gnu::dllexport]]` |
| The library, on Linux and macOS | `[[gnu::visibility("default")]]` |
| A host, a backend or a test | Nothing |

`docs/extern/bridge.h.md`, *The library*, tells which build defines `BRIDGE_BUILDING`.
`BACKEND_EXPORT` of `include/bridge_backend.h` is a different mark: a backend writes it before each of the 19 entries that its own library exports.
