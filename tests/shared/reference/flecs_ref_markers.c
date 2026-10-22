/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "flecs_ref.h"

static ecs_entity_t make_component(ecs_world_t *w)
{
    const ecs_component_desc_t desc = { .type = { .size = 4, .alignment = 4 } };

    return (ecs_component_init(w, &desc));
}

static void make_markers(ecs_world_t *w, ecs_entity_t *marker, bool tags)
{
    uint32_t k = 0;

    while (k < MARKERS) {
        marker[k] = tags ? ecs_new(w) : make_component(w);
        k++;
    }
}

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
