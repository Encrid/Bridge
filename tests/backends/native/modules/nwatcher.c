/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_native.h"
#include "game.h"
#include "native_idle.h"
#include "native_reach.h"

static uint32_t marked;
static uint32_t seeing;

static uint32_t follow(const bridge_api_t *api, uint64_t id)
{
    const bridge_header_t *h = bridge_native_get_header(api);
    const bridge_entity_t *e = get_entity_index(api) + (uint32_t)id;
    const marked_t *column;
    uint32_t base;

    if ((uint32_t)id >= h->entities || e->generation != (uint32_t)(id >> 32))
        return (0xdead);
    base = get_column_bases(api)[e->table * h->types + marked];
    if (!base)
        return (0xdead);
    column = (const marked_t *)(api->shared_base + base);
    return (column[e->row].by);
}

BRIDGE_NATIVE(seen)
{
    const bridge_list_t *list = bridge_native_get_list(api, seeing);
    const bridge_trigger_t *t;
    bridge_line_t line = { "seen", 4, 0, 0 };
    uint32_t i = 0;

    t = (const bridge_trigger_t *)(api->private_base + list->at);
    while (i < list->count) {
        line.a0 = t[i].events;
        line.a1 = follow(api, t[i].id);
        bridge_native_log(api, BRIDGE_LOG_INFO, &line);
        i++;
    }
    return (0);
}

BRIDGE_NATIVE(init)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    marked = BRIDGE_NATIVE_COMPONENT(api, "game:Marked", marked_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    terms[0].slot = marked;
    seeing = BRIDGE_NATIVE_OBSERVER(api, "seen", BRIDGE_ON_ENTER, terms, 1,
        nullptr, 0);
    return (0);
}
