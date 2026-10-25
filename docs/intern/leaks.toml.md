# src/leaks.toml

This file names the resource that a host takes from Bridge, for the count of `test --leaks` of the host.

`bridge_create` takes a world, and `bridge_destroy` gives it back.
A release ships the file as `leaks.toml`: `docs/extern/bridge.h.md`, *The library*.
`bridge_create` calls `bridge_destroy` when it fails, so the count of Bridge itself does not read this file.
