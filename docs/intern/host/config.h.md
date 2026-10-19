# src/host/config.h

This private header describes a parsed configuration and the keys that the host reads from it.
It defines the structs of the reader, the enums of the checks, and declares the `config_` functions.

## The data

`config_t` owns the array of `cfile_t`, the index of the local file, and a count of refusals.
A `cfile_t` keeps one file: its path, its folder, its whole path, its parsed `toml_t`, and the files that it names.
A `said_t` points at one value together with its file and line.
A `found_t` holds the chosen value beside a peer that clashes with it.

A `keyed_t` and a `module_key_t` name a key to search for.
A `names_t` gathers names without a repeat.
`enum check` names the kind of a key.
`value_rule_t` and `module_rule_t` drive the checks and the copy into Bridge.

## Functions

| Function | Result |
| --- | --- |
| `config_read`, `config_add_file`, `config_take_file` | Read the files of a configuration, and add one file. |
| `config_check`, `config_check_*`, `config_check_value` | Check a statement against the rule of its key. |
| `config_find`, `config_find_key`, `config_states_key`, `config_states_module` | Resolve a key over the files. |
| `config_names`, `names_add` | List the names under a key. |
| `config_say_*`, `config_path`, `config_get_folder`, `config_go_beside` | Print a key or a problem, and build paths. |
