/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_create.c -- a deck's storage made, and unmade.
*/

#include <stdlib.h>

#include "storage_create.h"

/*
** set_sizes
**
** Takes from the budget how much of each thing storage may hold. Table 0 is
** no table, and index 0 is no entity.
*/
static void set_sizes(storage_t *r, bridge_t *b)
{
    r->b = b;
    r->table_room = b->tables + b->sparse_sets;
    r->set_room = b->sparse_sets;
    r->direct_room = b->direct_indexed;
    r->bit_words = (b->entities + 63) / 64;
    r->table_high = 1;
    r->high = 1;
    r->words = (b->max_types + 63) / 64;
    r->type_room = r->words * 64;
    r->terms_room = b->max_systems;
}

static bool make_all(storage_t *r)
{
    if (!storage_make_tables(r) || !storage_make_fold(r))
        return (false);
    return (storage_make_apart(r) && storage_make_terms(r));
}

/*
** storage_create
**
** Makes a deck's storage: all of it, or none. `who` has a slot for each
** eight bytes of the column region: a table's ids sit in it, so its twin
** sits at the same place here. What is never touched is never backed by
** memory.
**
** failure.md, *Out of memory*
*/
storage_t *storage_create(bridge_t *b)
{
    storage_t *r = calloc(1, sizeof(*r));

    if (bridge_lacks(b, r, "storage"))
        return (nullptr);
    set_sizes(r, b);
    r->who = os_reserve(b->columns_len, b->columns_len);
    if (!make_all(r)) {
        (void)bridge_lacks(b, nullptr, "storage, as [budget] sizes it");
        storage_destroy(r);
        return (nullptr);
    }
    if (r->who)
        return (r);
    say_error(b, "[budget] cannot be satisfied: the operating system would "
        "not give storage %u bytes of its own for the columns it keeps",
        b->columns_len);
    storage_destroy(r);
    return (nullptr);
}

static void free_fold(storage_t *r)
{
    free(r->work);
    free(r->changed);
    free(r->folded);
    free(r->dest);
    free(r->attached);
    free(r->at);
    free(r->scratch);
    free(r->before);
    free(r->masked);
    free(r->in_table);
    free(r->set_type);
    free(r->set_table);
    free(r->where);
    free(r->direct_type);
    free(r->given);
}

/*
** storage_destroy
**
** Frees a deck's storage.
*/
void storage_destroy(storage_t *r)
{
    if (r->who)
        os_release(r->who, r->b->columns_len);
    free(r->types);
    free(r->type_pool);
    free(r->terms_pool);
    free(r->lists);
    free(r->term_types);
    free(r->term_opers);
    free(r->systems);
    free(r->observers);
    free(r->places);
    free(r->tables);
    free(r->table_next);
    free(r->dirty);
    free(r->hash);
    free(r->sets);
    free(r->columns);
    free_fold(r);
    free(r);
}
