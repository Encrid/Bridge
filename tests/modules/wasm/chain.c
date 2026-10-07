/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/chain.c -- a system adds A; an observer of A adds B;
** an observer of B adds C; an observer of C says so. One hop per barrier:
** three barriers, and no fixpoint.
*/

#include "bridge_wasm.h"
#include "chain.h"
#include "wasm_idle.h"

static uint32_t a;
static uint32_t b;
static uint32_t c;
static uint32_t on_a;
static uint32_t on_b;
static uint32_t started;
static uint64_t entity;

/*
** start
**
** Attaches A to the entity, on its first pass only.
*/
BRIDGE_EXPORT(start) int start(void)
{
    link_t link = { 1 };

    if (!started)
        BRIDGE_ATTACH(entity, a, link);
    started++;
    return (0);
}

/*
** got_a
**
** Says that A was added and, when the observer was triggered, attaches B.
*/
BRIDGE_EXPORT(got_a) int got_a(void)
{
    link_t link = { 2 };

    BRIDGE_LOG(BRIDGE_LOG_INFO, "hop A", 0, 0);
    if (bridge_count_triggers(on_a))
        BRIDGE_ATTACH(entity, b, link);
    return (0);
}

/*
** got_b
**
** Says that B was added and, when the observer was triggered, attaches C.
*/
BRIDGE_EXPORT(got_b) int got_b(void)
{
    link_t link = { 3 };

    BRIDGE_LOG(BRIDGE_LOG_INFO, "hop B", 0, 0);
    if (bridge_count_triggers(on_b))
        BRIDGE_ATTACH(entity, c, link);
    return (0);
}

/*
** got_c
**
** Says that C was added. It is the last hop, and attaches nothing.
*/
BRIDGE_EXPORT(got_c) int got_c(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "hop C", 0, 0);
    return (0);
}

/*
** init
**
** Declares the three components, the system that adds A and an observer of
** each component, each stating the component it adds, and spawns the
** entity.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    int none = 0;

    a = BRIDGE_COMPONENT("game:LinkA", link_t, BRIDGE_STRIP, BRIDGE_TABLE);
    b = BRIDGE_COMPONENT("game:LinkB", link_t, BRIDGE_STRIP, BRIDGE_TABLE);
    c = BRIDGE_COMPONENT("game:LinkC", link_t, BRIDGE_STRIP, BRIDGE_TABLE);
    BRIDGE_ADD_SYSTEM("start", "update", &none, 0, &a, 1);
    terms[0].slot = a;
    on_a = BRIDGE_OBSERVER("got_a", BRIDGE_ON_ENTER, terms, 1, &b, 1);
    terms[0].slot = b;
    on_b = BRIDGE_OBSERVER("got_b", BRIDGE_ON_ENTER, terms, 1, &c, 1);
    terms[0].slot = c;
    BRIDGE_OBSERVER("got_c", BRIDGE_ON_ENTER, terms, 1, nullptr, 0);
    entity = bridge_spawn();
    return (0);
}
