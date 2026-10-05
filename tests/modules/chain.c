/* A system adds A; an observer of A adds B; an observer of B adds C; an observer of C says so.
 * One hop per barrier: three barriers, and no fixpoint. */
#include "bridge-wasm.h"

ECS_STRUCT(Link, { uint32_t n; });

#define NONE 0xffffffffu

static uint32_t a, b, c, on_a, on_b, on_c, started;
static uint64_t entity;

static void hop(uint32_t observer, uint32_t next, uint32_t n)
{
    Link link = { n };
    if (bridge_triggers(observer) && next != NONE) BRIDGE_ATTACH(entity, next, link);
}

BRIDGE_EXPORT(start) int start()
{
    Link link = { 1 };
    if (!started ++) BRIDGE_ATTACH(entity, a, link);
    return 0;
}

BRIDGE_EXPORT(got_a) int got_a()
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "hop A", 0, 0);
    hop(on_a, b, 2);
    return 0;
}

BRIDGE_EXPORT(got_b) int got_b()
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "hop B", 0, 0);
    hop(on_b, c, 3);
    return 0;
}

BRIDGE_EXPORT(got_c) int got_c()
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "hop C", 0, 0);
    hop(on_c, NONE, 0);
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    bridge_term terms[1] = {{ 0, BRIDGE_IN, BRIDGE_AND, 0 }};
    int none = 0;
    a = BRIDGE_COMPONENT("game:LinkA", Link, BRIDGE_STRIP);
    b = BRIDGE_COMPONENT("game:LinkB", Link, BRIDGE_STRIP);
    c = BRIDGE_COMPONENT("game:LinkC", Link, BRIDGE_STRIP);
    BRIDGE_SYSTEM("start", "update", &none, 0);
    terms[0].slot = a;
    on_a = BRIDGE_OBSERVER("got_a", BRIDGE_ON_ADD, terms, 1);
    terms[0].slot = b;
    on_b = BRIDGE_OBSERVER("got_b", BRIDGE_ON_ADD, terms, 1);
    terms[0].slot = c;
    on_c = BRIDGE_OBSERVER("got_c", BRIDGE_ON_ADD, terms, 1);
    entity = bridge_spawn();
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
