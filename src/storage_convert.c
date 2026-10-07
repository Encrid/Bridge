/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage_convert.c -- the second half of a move: each entity placed
** once, its values taken before it leaves where they were and written
** where it is now.
*/

#include <stdlib.h>
#include <string.h>

#include "storage_convert.h"
#include "storage_forget.h"
#include "storage_place.h"

constexpr uint32_t MOVES_MOST = 64;

/*
** take_values
**
** Copies the values of the moved types that the entity at `index` has, one
** after another, and writes into `had` where each was: 0 for one it has not.
*/
static void take_values(const convert_t *c, uint32_t index, uint32_t *had)
{
    const storage_type_t *from;
    const uint8_t *shared = c->r->b->shared;
    uint32_t at = 0;
    uint32_t k = 0;

    while (k < c->count && k < MOVES_MOST) {
        from = c->moves[k].from;
        had[k] = get_value(c->r, index, from);
        if (had[k])
            memcpy(c->was + at, shared + had[k], from->size);
        at += from->size;
        k++;
    }
}

/*
** write_runs
**
** Writes the value of one moved type as its new type: zero, but for the runs
** of bytes copied across. `was` is the value as it was, and `to` where the
** new one is.
*/
static void write_runs(const convert_t *c, uint32_t k, const uint8_t *was,
    uint32_t to)
{
    const storage_move_t *move = &c->moves[k];
    uint8_t *value = c->r->b->shared + to;
    uint32_t run = 0;

    while (run < move->run_count) {
        memcpy(value + move->runs[run].to, was + move->runs[run].from,
            move->runs[run].len);
        run++;
    }
}

/*
** write_values
**
** Writes, for the entity at `index` now that it is placed, each value it
** had of a moved type, as the type it became. A singleton's is on index 0.
*/
static void write_values(const convert_t *c, uint32_t index,
    const uint32_t *had)
{
    const storage_move_t *move;
    uint32_t owner;
    uint32_t at = 0;
    uint32_t k = 0;

    while (k < c->count && k < MOVES_MOST) {
        move = &c->moves[k];
        owner = move->to->kept == BRIDGE_SINGLETON ? 0 : index;
        if (had[k])
            write_runs(c, k, c->was + at, get_value(c->r, owner, move->to));
        at += move->from->size;
        k++;
    }
}

/*
** convert_all
**
** Places each changed entity once, with its values carried across.
*/
static void convert_all(const convert_t *c)
{
    storage_t *r = c->r;
    uint32_t had[MOVES_MOST];
    uint32_t i = 0;

    while (i < r->changed_count) {
        memset(had, 0, sizeof(had));
        take_values(c, r->changed[i], had);
        place_one(r, r->changed[i]);
        write_values(c, r->changed[i], had);
        i++;
    }
}

/*
** storage_convert
**
** Writes the rows of components again, as other types. All of them or none.
*/
enum storage_moved storage_convert(storage_t *r, const storage_move_t *moves,
    uint32_t count)
{
    convert_t c = { r, moves, count, nullptr };
    uint32_t total = 0;
    uint32_t k = 0;

    while (k < count) {
        total += moves[k].from->size;
        k++;
    }
    c.was = malloc(total ? total : 1);
    if (!c.was)
        return (STORAGE_NO_MEMORY);
    if (!storage_fold_moves(&c)) {
        free(c.was);
        return (STORAGE_NO_ROOM);
    }
    convert_all(&c);
    forget_all(r);
    free(c.was);
    return (STORAGE_MOVED);
}
