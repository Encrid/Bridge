/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/reload_rows.c -- the rows of a type that a reload made another in the
** place of.
**
** lifetime.md, *When a component is moved*
*/

#include "reload.h"

/*
** set_move
**
** Writes the move of a type's rows to the one made in its place: the whole
** value, as many bytes as the shorter of the two, or, where the component
** layout changed, field by field, as declare worked it out.
*/
static void set_move(const declared_type_t *c, storage_move_t *move,
    storage_run_t *whole)
{
    const declared_type_t *was = c->replaces;
    uint32_t size = c->size < was->size ? c->size : was->size;

    *whole = (storage_run_t){ 0, 0, size };
    *move = (storage_move_t){ was->type, c->type, whole, 1 };
    if (!was->runs)
        return;
    move->runs = was->runs;
    move->run_count = was->run_count;
}

/*
** settle
**
** The rows have moved: no type replaces another any more, and what went is
** said, by the names it was accepted under.
**
** lifetime.md, *When a component layout changed*
*/
static void settle(bridge_t *b, const module_t *n)
{
    say_t about = { n, nullptr, BRIDGE_WARN };
    uint32_t i = 0;

    while (i < n->declared_count) {
        n->declared[i]->replaces = nullptr;
        i++;
    }
    i = 0;
    while (i < n->loss_count) {
        say_about(b, &about, "module '%s' is reloaded, and '%s' is lost from "
            "%u rows, as it accepted", n->identity, n->losses[i],
            n->loss_rows[i] - 1);
        i++;
    }
}

static uint32_t list_moves(const module_t *n, storage_move_t *moves,
    storage_run_t *whole)
{
    uint32_t count = 0;
    uint32_t i = 0;

    while (i < n->declared_count && count < 64) {
        if (n->declared[i]->replaces)
            set_move(n->declared[i], &moves[count], &whole[count]);
        count += n->declared[i]->replaces != nullptr;
        i++;
    }
    return (count);
}

/*
** refuse_move
**
** Says why the rows of `count` components were not moved: for want of the
** memory to carry their values, or of room where they were to go.
**
** lifetime.md, *When a component is moved*
** failure.md, *Out of memory*
*/
static int refuse_move(reloading_t *r, enum storage_moved moved,
    uint32_t count)
{
    const bridge_t *b = r->b;

    if (moved == STORAGE_NO_MEMORY)
        return (module_fail(&r->err, "out of memory: there was none to "
            "carry across the values of what it keeps another way, %u of "
            "its components", count));
    return (module_fail(&r->err, "what it keeps another way, %u of its "
        "components, could not be moved: [budget] tables of %u or [budget] "
        "columns of %u bytes has no room for the rows where they are to go, "
        "while they are still where they were", count, b->tables,
        b->columns_len));
}

/*
** reload_move_rows
**
** Every type this reload made in the place of another takes that one's
** rows, all of them or none.
*/
int reload_move_rows(reloading_t *r)
{
    const module_t *n = r->n;
    storage_move_t moves[64];
    storage_run_t whole[64];
    uint32_t count = list_moves(n, moves, whole);
    enum storage_moved moved = STORAGE_MOVED;

    if (count)
        moved = storage_convert(r->b->storage, moves, count);
    if (moved != STORAGE_MOVED)
        return (refuse_move(r, moved, count));
    settle(r->b, n);
    return (0);
}
