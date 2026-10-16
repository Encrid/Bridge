# src/bridge_backend.c

This file opens one backend of a deployment: it opens the library of the backend, and starts the backend with its settings.

## A backend of the deployment

A backend has a name and a library, and no two backends have the same name.
Bridge opens the library by the path that the deployment states, with `backend_open` of `src/backend.c`.
A library named by a path is looked for at that path, and what it needs is looked for beside it.
A bare name is left to the search of the operating system: `src/os/windows/library.c` and `src/os/posix/library.c`.
The deck counts the backend as soon as it has its name, so that a deck that is refused frees it.
Then `backend_create` starts the backend with its settings.

## The settings of a backend

The settings of a backend are its own, for example a cache folder, and Bridge passes them to `backend_create` unread.
A backend refuses a setting that it does not know, by name, and then the deck is not created.
The host seam states a setting as `bridge_setting_t`, and the backend seam as `backend_setting_t`.
They are two structs for one fact, so that neither header needs the other.
Static assertions hold them to one shape: the same four kinds with the same numbers, the same size and the same offsets.
`convert_settings` copies the settings of the host seam into the structs of the backend seam.

## Functions

| Function | Result |
| --- | --- |
| `check_backend` | Refuses a backend with no name or no library, or with the name of a backend before it. 1, with a record, if refused. |
| `convert_settings` | Gives a new array of the settings of a backend, as the backend seam states them. Null if there is not the memory. |
| `open_library` | Copies the name of the backend, counts the backend, opens its library and finds its entries. 1, with a record, if it fails. |
| `start_backend` | Creates the backend with its settings. 1, with a record, if there is not the memory or the backend does not start. |
| `bridge_open_backend` | Opens the backend `i` of the deployment and starts it. 1, with a record, if one step fails. |
