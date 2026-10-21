/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "bits.h"
#include "storage_create.h"

static void set_sizes(storage_t *r, bridge_world_t *w)
{
    r->w = w;
    r->table_room = w->tables + w->sparse_sets;
    r->set_room = w->sparse_sets;
    r->direct_room = w->direct_indexed;
    r->bit_words = bits_words(w->entities);
    r->table_high = 1;
    r->high = 1;
    r->words = bits_words(w->max_types);
    r->type_room = r->words * 64;
    r->terms_room = w->max_systems;
}

static bool make_all(storage_t *r)
{
    if (!storage_make_tables(r) || !storage_make_fold(r))
        return (false);
    return (storage_make_apart(r) && storage_make_terms(r));
}

storage_t *storage_create(bridge_world_t *w)
{
    storage_t *r = calloc(1, sizeof(*r));

    if (bridge_lacks(w, r, "storage"))
        return (nullptr);
    set_sizes(r, w);
    r->who = os_reserve(w->columns_len, w->columns_len);
    if (!make_all(r)) {
        (void)bridge_lacks(w, nullptr, "storage, as [budget] sizes it");
        storage_destroy(r);
        return (nullptr);
    }
    if (r->who)
        return (r);
    say_error(w, "[budget] cannot be satisfied: the operating system would "
        "not give storage %u bytes of its own for the columns it keeps",
        w->columns_len);
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

void storage_destroy(storage_t *r)
{
    if (r->who)
        os_release(r->who, r->w->columns_len);
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
