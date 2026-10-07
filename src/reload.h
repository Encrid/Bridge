/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/reload.h -- a module being reloaded: what the steps of a reload
** share.
**
** lifecycle.md, *The runtime lifecycle*
*/

#ifndef BRIDGE_SRC_RELOAD_H
    #define BRIDGE_SRC_RELOAD_H

    #include "module.h"

/*
** A reload in progress.
**
**     on, demands   the backend the artifact names, and what its config
**                   asks of it
**     old, n        the instance that is loaded, and the one to take its
**                   place
**     extra         how many bytes its share of the persistent arena grows
**                   by
**     in_place      that its last range grew where it stands, which was
**                   `last`; otherwise the bytes are a range more, at
**                   `extra_at`
**     side_by_side  that the backend can hold both instances at once
**     err           why it failed
*/
typedef struct {
    bridge_t *b;
    const bridge_artifact_t *a;
    open_backend_t *on;
    enum backend_capability demands;
    module_t *old;
    module_t *n;
    uint32_t extra;
    uint32_t extra_at;
    range_t last;
    bool in_place;
    bool side_by_side;
    backend_err_t err;
} reloading_t;

/*
** reload_refuse
**
** Says that the loaded module is not reloaded, and why, written as printf
** would. Answers 1.
*/
[[gnu::format(printf, 2, 3)]] int reload_refuse(const reloading_t *r,
    const char *format, ...);

/*
** reload_check
**
** Refuses a reload of a module that is not loaded, that may not be
** reloaded, or that names another backend than it runs on. 1 with a record.
*/
[[nodiscard]] int reload_check(reloading_t *r);

/*
** reload_grow_share
**
** Grows a module's share of the persistent arena to what the reload states.
** A budget changes by reload, and this one only upwards: what the module
** has allocated is in its share, and stays there for as long as it is
** loaded. 1 with a record.
**
** budgets.md, *Budgets*
*/
[[nodiscard]] int reload_grow_share(reloading_t *r);

/*
** reload_give_back
**
** Gives back what reload_grow_share added: nothing was allocated from it
** that lived.
*/
void reload_give_back(reloading_t *r);

/*
** reload_make_record
**
** Makes the new instance's record, which takes over what was set aside for
** the module. Until the swap these are copies: a reload that fails leaves
** the old record's own as they were. 1 with a record, and nothing of the
** old instance touched.
**
** failure.md, *Out of memory*
*/
[[nodiscard]] int reload_make_record(reloading_t *r);

/*
** reload_move_rows
**
** Every type this reload made in the place of another takes that one's
** rows, all of them or none. 1 and why if there is no room.
**
** lifetime.md, *When a component is moved*
*/
[[nodiscard]] int reload_move_rows(reloading_t *r);

/*
** reload_fail
**
** Says that a reload failed and takes the new instance back out. The old
** one is kept if there is one left to keep, and otherwise the module stays,
** stopped. Answers 1.
**
** failure.md, *When something fails*
*/
int reload_fail(reloading_t *r);

#endif
