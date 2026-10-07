/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/ntag/ntag_look_set.c -- what a module is handed for
** a tag kept in a sparse set.
*/

#include "ntag.h"

/*
** look_index
**
** Says how many entities have a cell of P's index that says carried.
*/
static void look_index(const bridge_api_t *api)
{
    const uint32_t *index = bridge_native_get_sparse_index(api, ntag_p);
    uint32_t carried = 0;
    uint32_t i = 0;

    while (i < COUNT) {
        carried += index[(uint32_t)ntag_ids[i]] == BRIDGE_CARRIED;
        i++;
    }
    say(api, "P, by its index: entities whose cell says carried", carried, 0);
}

/*
** ntag_look_set
**
** Says what a module is handed for P, the tag kept in a sparse set: by the
** spans of a query on it alone, whose base says carried as a table's does,
** and by each entity's cell of its index, which says the same.
*/
void ntag_look_set(const bridge_api_t *api)
{
    const bridge_query_t *q = bridge_native_get_query(api, ntag_in_set);
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t carried = 0;
    uint32_t i = 0;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 1);
        rows += span[0];
        carried += span[2] == BRIDGE_CARRIED;
        i++;
    }
    say(api, "P, in a sparse set: rows, and spans whose base says carried",
        rows, carried);
    look_index(api);
}
