# src/host/host.h

This private header gathers the whole host.
It sets the constant `FILES`, the `arrive` choices, and the structs that one run shares.
It then declares the functions that the other sources of `src/host` define.

## The data

`host_t` holds one run: the parsed `config_t`, the world settings, the backends, the content paths, the live `bridge_t`, and a fixed array of `file_t` slots.
A `file_t` records one known file by its path, its hash mark, and the module behind it.
A `handed_t` keeps the bytes of a file together with the manifest that the host read from them.
A `held_t` names a backend, its library path, and its arrive rule.
`stating_t`, `look_t` and `seeing_t` carry state while the host states a module or walks the content.

## Functions

| Function | Result |
| --- | --- |
| `config_world`, `config_order`, `config_backends`, `config_content` | Take the world, order, backend and content keys. |
| `config_load`, `config_unfound`, `get_held` | Read `content.load`, report named modules that no look finds, and find a backend. |
| `run`, `look`, `walk_content` | Run the world, do one look, and walk the content paths. |
| `find_file`, `meet`, `may_arrive`, `load_found`, `make_artifact` | Track a slot, read a met file, ask to trust it, load it, and build its artifact. |
| `show_config`, `show_module`, `take_lock` | Show a configuration, show one module, and take the run lock. |
| `state_*`, `read_own` | State the keys, lists, phases and settings of a module. |
