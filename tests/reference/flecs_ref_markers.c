/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/reference/flecs_ref_markers.c -- what the process holds for 204,800
** entities with a four-byte value and six markers each of twelve, over
** 4,096 tables.
*/

#include <stdio.h>

#include "flecs_ref.h"

/*
** make_component
**
** Answers a new component of four bytes, with no name.
*/
static ecs_entity_t make_component(ecs_world_t *w)
{
    const ecs_component_desc_t desc = { .type = { .size = 4, .alignment = 4 } };

    return (ecs_component_init(w, &desc));
}

/*
** make_markers
**
** Makes the twelve markers: tags, or components of four bytes.
*/
static void make_markers(ecs_world_t *w, ecs_entity_t *marker, bool tags)
{
    uint32_t k = 0;

    while (k < MARKERS) {
        marker[k] = tags ? ecs_new(w) : make_component(w);
        k++;
    }
}

/*
** mark_entity
**
** Gives an entity each marker whose bit is set in `bits`.
*/
static void mark_entity(ecs_world_t *w, ecs_entity_t e,
    const ecs_entity_t *marker, uint32_t bits)
{
    uint32_t k = 0;

    while (k < MARKERS) {
        if (bits >> k & 1)
            ecs_add_id(w, e, marker[k]);
        k++;
    }
}

/*
** make_entities
**
** Makes ROWS entities, each with the value and the markers its number
** sets: every one of the 4,096 sets of twelve markers, fifty times.
*/
static void make_entities(ecs_world_t *w, ecs_entity_t value,
    const ecs_entity_t *marker)
{
    ecs_entity_t e;
    int32_t i = 0;

    while (i < ROWS) {
        e = ecs_new(w);
        ecs_add_id(w, e, value);
        mark_entity(w, e, marker, (uint32_t)i % 4096);
        i++;
    }
}

/*
** flecs_ref_measure_markers
**
** Says how many kilobytes more the process holds once the entities are
** made than before the world was.
*/
void flecs_ref_measure_markers(bool tags)
{
    ecs_world_t *w;
    ecs_entity_t value;
    ecs_entity_t marker[MARKERS];
    size_t before = flecs_ref_read_held();

    w = ecs_mini();
    value = make_component(w);
    make_markers(w, marker, tags);
    make_entities(w, value, marker);
    printf("flecs, 204800 entities with six markers each, as %s: the "
        "process holds %zu KB more\n",
        tags ? "tags" : "components of four bytes",
        flecs_ref_read_held() - before);
    ecs_fini(w);
}
