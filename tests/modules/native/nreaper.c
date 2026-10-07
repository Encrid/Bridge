/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nreaper.c -- a system that destroys every entity
** with a position, and says how many it found. Whether it may is the
** deployment's: with a read grant it has no stake in them and they stay,
** with a write grant they go.
**
** grants.md, *What a module may destroy*
*/

#include "bridge_native.h"
#include "game.h"
#include "native_idle.h"

static uint32_t position;
static uint32_t reaping;

static void destroy_rows(const bridge_api_t *api, const uint32_t *span)
{
    const uint64_t *who = (const uint64_t *)(api->shared_base + span[1]);
    uint32_t row = 0;

    while (row < span[0]) {
        api->destroy(api, who[row]);
        row++;
    }
}

/*
** reap
**
** Destroys every entity its query matched, and says how many rows that
** was.
*/
BRIDGE_NATIVE(reap)
{
    const bridge_query_t *q = bridge_native_get_query(api, reaping);
    bridge_line_t line = { "rows", 4, 0, 0 };
    const uint32_t *span;
    uint32_t s = 0;

    while (s < q->spans) {
        span = bridge_native_get_span(api, q, s, 1);
        destroy_rows(api, span);
        line.a0 += span[0];
        s++;
    }
    bridge_native_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

/*
** init
**
** Declares the position and the system, which reads it.
*/
BRIDGE_NATIVE(init)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    position = BRIDGE_NATIVE_COMPONENT(api, "game:Position", position_t,
        BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = position;
    reaping = BRIDGE_NATIVE_ADD_SYSTEM(api, "reap", "update", terms, 1,
        nullptr, 0);
    return (0);
}
