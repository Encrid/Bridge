/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "reload.h"

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
