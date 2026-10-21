/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "bridge_inside.h"
#include "reload.h"

static uint32_t count_replaced(const module_t *n)
{
    uint32_t j = 0;
    uint32_t c = 0;

    while (j < n->declared_count) {
        c += n->declared[j]->replaces != nullptr;
        j++;
    }
    return (c);
}

static uint32_t fill_from(const module_t *n, storage_move_t *moves,
    storage_run_t *whole, uint32_t k)
{
    uint32_t j = 0;

    while (j < n->declared_count) {
        if (n->declared[j]->replaces)
            reload_set_move(n->declared[j], &moves[k], &whole[k]);
        k += n->declared[j]->replaces != nullptr;
        j++;
    }
    return (k);
}

static enum storage_moved do_moves(reloading_t *reloads, uint32_t count,
    uint32_t total)
{
    storage_move_t *moves = calloc(total, sizeof(*moves));
    storage_run_t *whole = calloc(total, sizeof(*whole));
    enum storage_moved moved = STORAGE_NO_MEMORY;
    uint32_t k = 0;
    uint32_t i = 0;

    while (moves && whole && i < count) {
        k = fill_from(reloads[i].n, moves, whole, k);
        i++;
    }
    if (moves && whole)
        moved = storage_convert(reloads[0].w->storage, moves, total);
    free(moves);
    free(whole);
    return (moved);
}

int reload_move_group(reloading_t *reloads, uint32_t count, char *why,
    size_t len)
{
    enum storage_moved moved;
    uint32_t total = 0;
    uint32_t i = 0;

    while (i < count) {
        total += count_replaced(reloads[i].n);
        i++;
    }
    moved = total ? do_moves(reloads, count, total) : STORAGE_MOVED;
    if (moved != STORAGE_MOVED) {
        reload_refuse_move(&reloads[0], moved, total);
        snprintf(why, len, "%s", reloads[0].err.text);
        return (1);
    }
    for (i = 0; i < count; i++)
        reload_settle(reloads[i].w, reloads[i].n);
    return (0);
}

void reload_commit_group(bridge_world_t *w, reloading_t *reloads,
    uint32_t count)
{
    uint32_t i = 0;

    while (i < count) {
        reload_swap(&reloads[i]);
        bridge_resume(w, reloads[i].n);
        i++;
    }
}
