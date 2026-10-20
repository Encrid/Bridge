/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "storage_create.h"

bool storage_make_tables(storage_t *r)
{
    const bridge_world_t *w = r->w;
    size_t tables = (size_t)r->table_room + 1;

    r->hash_room = 16;
    while (r->hash_room < tables * 2)
        r->hash_room *= 2;
    r->places = calloc(w->entities, sizeof(*r->places));
    r->tables = calloc(tables, sizeof(*r->tables));
    r->table_next = calloc(tables, sizeof(*r->table_next));
    r->dirty = calloc(tables, sizeof(*r->dirty));
    r->hash = calloc(r->hash_room, sizeof(*r->hash));
    r->sets = calloc(tables * r->words, 8);
    r->columns = calloc(tables * r->type_room, 4);
    if (!r->places || !r->tables || !r->table_next || !r->dirty)
        return (false);
    return (r->hash && r->sets && r->columns);
}

bool storage_make_fold(storage_t *r)
{
    size_t entities = r->w->entities;

    r->work = calloc(entities * r->words, 8);
    r->changed = calloc(entities, sizeof(*r->changed));
    r->folded = calloc(entities, 1);
    r->dest = calloc(entities, sizeof(*r->dest));
    r->attached = calloc(entities * r->words, 8);
    r->at = calloc(r->type_room, sizeof(*r->at));
    r->scratch = calloc(r->words, 8);
    r->before = calloc(r->words, 8);
    r->masked = calloc(r->words, 8);
    r->in_table = calloc(r->words, 8);
    if (!r->work || !r->changed || !r->folded || !r->dest || !r->attached)
        return (false);
    return (r->at && r->scratch && r->before && r->masked && r->in_table);
}

bool storage_make_apart(storage_t *r)
{
    size_t entities = r->w->entities;
    size_t sets = r->set_room ? r->set_room : 1;
    size_t direct = r->direct_room ? r->direct_room : 1;

    r->set_type = calloc((size_t)r->set_room + 1, sizeof(*r->set_type));
    r->set_table = calloc((size_t)r->set_room + 1, sizeof(*r->set_table));
    r->where = calloc(sets * entities, sizeof(*r->where));
    r->direct_type = calloc((size_t)r->direct_room + 1,
        sizeof(*r->direct_type));
    r->given = calloc(direct * r->bit_words, 8);
    if (!r->set_type || !r->set_table || !r->where)
        return (false);
    return (r->direct_type && r->given);
}

bool storage_make_terms(storage_t *r)
{
    size_t room = r->terms_room;
    size_t terms = room * r->w->max_terms;

    r->types = calloc((size_t)r->type_room + 1, sizeof(*r->types));
    r->type_pool = calloc(r->type_room, sizeof(*r->type_pool));
    r->terms_pool = calloc(room + 1, sizeof(*r->terms_pool));
    r->lists = calloc(room * 2,
        ((size_t)r->table_room + 2) * sizeof(*r->lists));
    r->term_types = calloc(terms, sizeof(*r->term_types));
    r->term_opers = calloc(terms, sizeof(*r->term_opers));
    r->systems = calloc(room, sizeof(*r->systems));
    r->observers = calloc(room, sizeof(*r->observers));
    if (!r->types || !r->type_pool || !r->terms_pool || !r->lists)
        return (false);
    return (r->term_types && r->term_opers && r->systems && r->observers);
}
