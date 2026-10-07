/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/native/nstamp.c -- a system that attaches game:Marked, with
** VALUE in it, to every entity that has a position. Two of these in one
** phase change one component of the same entities. PHASE is the phase it
** names: its author's word, which a deployment may call otherwise. Built
** with UNSTATED, it attaches without having said so.
**
** schedule.md, *Order within a phase is never implicit*
*/

#include "bridge_native.h"
#include "game.h"
#include "native_idle.h"

#ifdef UNSTATED
constexpr uint32_t STATED = 0;
#else
constexpr uint32_t STATED = 1;
#endif

static uint32_t position;
static uint32_t marked;
static uint32_t stamping;

/*
** stamp
**
** Attaches the mark, with VALUE in it, to every entity that has a position.
*/
BRIDGE_NATIVE(stamp)
{
    const bridge_query_t *q = bridge_native_get_query(api, stamping);
    marked_t m = { VALUE };
    const uint32_t *span;
    const uint64_t *who;
    uint32_t s = 0;
    uint32_t row;

    while (s < q->spans) {
        span = bridge_native_get_span(api, q, s, 1);
        who = (const uint64_t *)(api->shared_base + span[1]);
        row = 0;
        while (row < span[0]) {
            BRIDGE_NATIVE_ATTACH(api, who[row], marked, m);
            row++;
        }
        s++;
    }
    return (0);
}

/*
** init
**
** Declares the two components and the system, which states that it changes
** the mark unless the build is UNSTATED, and spawns one entity with a
** position.
*/
BRIDGE_NATIVE(init)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    position_t p = { 1.0f, 0.0f };

    position = BRIDGE_NATIVE_COMPONENT(api, "game:Position", position_t,
        BRIDGE_DELETE, BRIDGE_TABLE);
    marked = BRIDGE_NATIVE_COMPONENT(api, "game:Marked", marked_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    terms[0].slot = position;
    stamping = BRIDGE_NATIVE_ADD_SYSTEM(api, "stamp", PHASE, terms, 1,
        &marked, STATED);
    BRIDGE_NATIVE_ATTACH(api, api->spawn(api), position, p);
    return (0);
}
