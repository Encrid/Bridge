# src/host/host_run.c

This file makes the Bridge world and runs the rounds.

## The run

`make_world` fills a `bridge_config_t` and calls `bridge_create` with the record sink.
`run` loops the rounds while each of these holds:

- No call was refused, and no module asked for the end.
- The round limit holds, or `rounds` is 0.
- The run is live, or the last round had work: its look did work, or `say_record` counted a record in `said`.

`run_round` does a look, checks the named modules after the first look, runs each phase, and counts the round in `round`.
A phase that gives 2 sets `ended`, and the other phases of the round still run.
After the loop, `run` writes the line of an end that a module asked for, or else the line of the round limit.
`say_record` prints a record below the info level and counts an error. `say_loaded` lists the modules that stay loaded.

## Functions

| Function | Result |
| --- | --- |
| `run` | Runs the world to its end. Gives 0, or 1 on a failure. |
