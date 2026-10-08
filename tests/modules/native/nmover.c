/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nmover.c -- mover.c as a shared library: it reads
** and writes the column through a raw pointer.
*/

#include "bridge_native.h"
#include "game.h"
#include "native_idle.h"

constexpr uint32_t ENTITIES = 1000;

static uint32_t position;
static uint32_t marked;
static uint32_t moving;
static uint32_t tallying;

/*
** move
**
** Adds one to the x of every position.
*/
BRIDGE_NATIVE(move)
{
    const bridge_query_t *q = bridge_native_get_query(api, moving);
    const uint32_t *span;
    position_t *p;
    uint32_t s = 0;
    uint32_t r;

    while (s < q->spans) {
        span = bridge_native_get_span(api, q, s, 1);
        p = (position_t *)(api->shared_base + span[2]);
        r = 0;
        while (r < span[0]) {
            p[r].x += 1.0f;
            r++;
        }
        s++;
    }
    return (0);
}

/*
** tally
**
** Says what its own query matched.
*/
BRIDGE_NATIVE(tally)
{
    const bridge_query_t *q = bridge_native_get_query(api, tallying);
    bridge_line_t line = { "native tally", 12, q->spans, 0 };
    uint32_t s = 0;

    while (s < q->spans) {
        line.a1 += bridge_native_get_span(api, q, s, 1)[0];
        s++;
    }
    bridge_native_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

/*
** populate
**
** Spawns the entities: each with a position, and every other one marked.
*/
static void populate(const bridge_api_t *api)
{
    position_t p = { 0.0f, 0.0f };
    marked_t m = {};
    uint64_t id;

    while (m.by < ENTITIES) {
        id = api->spawn(api);
        p.x = (float)m.by;
        BRIDGE_NATIVE_ATTACH(api, id, position, p);
        if (m.by % 2)
            BRIDGE_NATIVE_ATTACH(api, id, marked, m);
        m.by++;
    }
}

/*
** init
**
** Declares the two components and the two systems, and spawns the entities.
*/
BRIDGE_NATIVE(init)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};

    position = BRIDGE_NATIVE_COMPONENT(api, "game:Position", position_t,
        BRIDGE_DELETE, BRIDGE_TABLE);
    marked = BRIDGE_NATIVE_COMPONENT(api, "game:Marked", marked_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    terms[0].slot = position;
    moving = BRIDGE_NATIVE_ADD_SYSTEM(api, "move", "update", terms, 1,
        nullptr, 0);
    terms[0].access = BRIDGE_READ;
    tallying = BRIDGE_NATIVE_ADD_SYSTEM(api, "tally", "present", terms, 1,
        nullptr, 0);
    populate(api);
    return (0);
}
