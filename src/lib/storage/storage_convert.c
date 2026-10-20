/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "storage_convert.h"
#include "storage_forget.h"
#include "storage_place.h"

static void take_values(const convert_t *c, uint32_t index, uint32_t *had)
{
    const storage_type_t *from;
    const uint8_t *shared = c->r->w->shared;
    uint32_t at = 0;
    uint32_t k = 0;

    while (k < c->count) {
        from = c->moves[k].from;
        had[k] = get_value(c->r, index, from);
        if (had[k])
            memcpy(c->was + at, shared + had[k], from->size);
        at += from->size;
        k++;
    }
}

static void write_runs(const convert_t *c, uint32_t k, const uint8_t *was,
    uint32_t to)
{
    const storage_move_t *move = &c->moves[k];
    uint8_t *value = c->r->w->shared + to;
    uint32_t run = 0;

    while (run < move->run_count) {
        memcpy(value + move->runs[run].to, was + move->runs[run].from,
            move->runs[run].len);
        run++;
    }
}

static void write_values(const convert_t *c, uint32_t index,
    const uint32_t *had)
{
    const storage_move_t *move;
    uint32_t owner;
    uint32_t at = 0;
    uint32_t k = 0;

    while (k < c->count) {
        move = &c->moves[k];
        owner = move->to->kept == BRIDGE_SINGLETON ? 0 : index;
        if (had[k])
            write_runs(c, k, c->was + at, get_value(c->r, owner, move->to));
        at += move->from->size;
        k++;
    }
}

static void convert_all(const convert_t *c)
{
    storage_t *r = c->r;

    for (uint32_t i = 0; i < r->changed_count; i++) {
        memset(c->had, 0, c->count * sizeof(*c->had));
        take_values(c, r->changed[i], c->had);
        place_one(r, r->changed[i]);
        write_values(c, r->changed[i], c->had);
    }
}

enum storage_moved storage_convert(storage_t *r, const storage_move_t *moves,
    uint32_t count)
{
    convert_t c = { r, moves, count, nullptr, nullptr };
    size_t total = 0;

    for (uint32_t k = 0; k < count; k++)
        total += moves[k].from->size;
    c.had = malloc(count * sizeof(*c.had) + total);
    if (!c.had)
        return (STORAGE_NO_MEMORY);
    c.was = (uint8_t *)(c.had + count);
    if (!storage_fold_moves(&c)) {
        free(c.had);
        return (STORAGE_NO_ROOM);
    }
    convert_all(&c);
    forget_all(r);
    free(c.had);
    return (STORAGE_MOVED);
}
