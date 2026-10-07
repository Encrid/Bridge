/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/storage_named.h -- a table found by its set, and listed by the
** systems that read it.
*/

#ifndef BRIDGE_SRC_STORAGE_NAMED_H
    #define BRIDGE_SRC_STORAGE_NAMED_H

    #include "storage_table.h"

/*
** name_table
**
** Makes table `t` the table its set is found as.
*/
static inline void name_table(storage_t *r, uint32_t t)
{
    uint32_t i = hash_set(r, get_set(r, t));

    while (r->hash[i])
        i = (i + 1) & (r->hash_room - 1);
    r->hash[i] = t;
    r->tables[t].named = true;
}

/*
** move_home
**
** Puts the table found at place `k` of the hash back where it is found now
** that a place before it is empty.
*/
static inline void move_home(storage_t *r, uint32_t k)
{
    uint32_t other = r->hash[k];
    uint32_t home = hash_set(r, get_set(r, other));

    r->hash[k] = 0;
    while (r->hash[home])
        home = (home + 1) & (r->hash_room - 1);
    r->hash[home] = other;
}

/*
** unname_table
**
** Makes table `t` the table its set is found as no longer.
*/
static inline void unname_table(storage_t *r, uint32_t t)
{
    uint32_t mask = r->hash_room - 1;
    uint32_t i = hash_set(r, get_set(r, t));
    uint32_t k;

    while (r->hash[i] != t)
        i = (i + 1) & mask;
    r->hash[i] = 0;
    k = (i + 1) & mask;
    while (r->hash[k]) {
        move_home(r, k);
        k = (k + 1) & mask;
    }
    r->tables[t].named = false;
}

/*
** list_table
**
** Puts table `t` in the list of each system that reads it.
*/
static inline void list_table(storage_t *r, uint32_t t)
{
    storage_terms_t *s;
    uint32_t i = 0;

    while (i < r->system_count) {
        s = r->systems[i];
        if (terms_read(r, s, t)) {
            s->list[s->matched] = t;
            s->matched++;
            s->pos[t] = s->matched;
            s->changed = true;
        }
        i++;
    }
}

/*
** unlist_table
**
** Takes table `t` out of every system's list: the last of a list takes its
** place.
*/
static inline void unlist_table(storage_t *r, uint32_t t)
{
    storage_terms_t *s;
    uint32_t at;
    uint32_t i = 0;

    while (i < r->system_count) {
        s = r->systems[i];
        at = s->pos[t];
        if (at) {
            s->matched--;
            s->list[at - 1] = s->list[s->matched];
            s->pos[s->list[at - 1]] = at;
            s->pos[t] = 0;
            s->changed = true;
        }
        i++;
    }
}

#endif
