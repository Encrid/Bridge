/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/bridge_inside.h -- a bridge made, stopped and run: what the files that
** do it share, and what the rest of the library asks of them.
**
** create.md, *Create*
*/

#ifndef BRIDGE_SRC_BRIDGE_INSIDE_H
    #define BRIDGE_SRC_BRIDGE_INSIDE_H

    #include "inside.h"

/*
** bridge_open_backends
**
** Opens every backend the deployment configures, starts it, and takes what
** it needs of a block it maps. 1, with a record, if one cannot be.
*/
[[nodiscard]] int bridge_open_backends(bridge_t *b, const bridge_config_t *cfg);

/*
** bridge_open_backend
**
** Opens the backend `i` of the deployment and starts it. 1, with a record,
** if it has no name or no library, is named twice, will not open or will not
** start.
*/
[[nodiscard]] int bridge_open_backend(bridge_t *b, const bridge_config_t *cfg,
    uint32_t i);

/*
** bridge_name_phases
**
** Takes the deployment's phases, each a name. 1, with a record, if it
** declares none, or one has no name or is named twice.
**
** schedule.md, *Bridge-ASM700 ships no phase names*
*/
[[nodiscard]] int bridge_name_phases(bridge_t *b, const bridge_config_t *cfg);

/*
** bridge_get_stem_length
**
** Answers how long an identity is without its version: up to its '@'.
*/
size_t bridge_get_stem_length(const char *identity);

/*
** bridge_find_pinned
**
** Answers a loaded module that is pinned to another thread than the calling
** one, or nullptr. A pinned module's code runs on the thread that loaded it,
** or not at all.
**
** pinned.md, *Threads*
*/
const module_t *bridge_find_pinned(const bridge_t *b);

/*
** bridge_check_call
**
** Answers 1, with a record, if nothing may run: the bridge has stopped, or a
** module is pinned to another thread. `repairing` names a module being
** loaded or reloaded: the one call a stopped bridge accepts is one that
** brings back the module that stopped it.
*/
[[nodiscard]] int bridge_check_call(bridge_t *b, const char *repairing);

/*
** bridge_stop
**
** Stops the bridge: a module stated `fail` is lost, and nothing runs until
** it is back. `what` says what became of it.
**
** failure.md, *When something fails*
*/
void bridge_stop(bridge_t *b, const char *identity, const char *what);

/*
** bridge_resume
**
** Lets the bridge run again, if `m` is the module it was waiting for.
*/
void bridge_resume(bridge_t *b, const module_t *m);

/*
** bridge_give_up
**
** Lets the bridge run again without the module that stopped it: the host
** unloads it, by name, and so says the deployment goes on without it.
*/
void bridge_give_up(bridge_t *b, const module_t *m);

#endif
