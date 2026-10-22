# src/host/config/config_backend.c

This file takes the backends and their libraries into the host.

`config_backends` lists the names under `backend`, then takes each one.
`take_backend` reads the library under `backend.<name>` and the word under `content.arrive.<name>`.
`with_suffix` appends the platform library suffix when the path lacks it.
`get_arrive` maps `ask`, `load` or `leave` onto an `arrive` value.
`get_held` looks a held backend up by name.

## Functions

| Function | Result |
| --- | --- |
| `config_backends` | Fills the backend and held arrays. Prints `not stated: backend` when none exist. |
| `get_held` | Gives the held backend of a name, or null. |
