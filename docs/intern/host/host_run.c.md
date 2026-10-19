# src/host/host_run.c

This file makes the Bridge world and runs the rounds.

## The run

`make_world` fills a `bridge_config_t` and calls `bridge_create` with the record sink.
`run` loops the rounds while work remains and the round limit holds.
`run_round` does a look, checks the named modules after the first look, and runs each phase.
`say_record` prints a record below the info level and counts an error. `say_loaded` lists the modules that stay loaded.

## Functions

| Function | Result |
| --- | --- |
| `run` | Runs the world to its end. Gives 0, or 1 on a failure. |
