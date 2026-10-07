/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/nstress/nstress.c -- storage under load, with the
** module's own account of what should be there.
**
** Each pass it asks for twenty thousand changes chosen by a generator it
** seeds itself: an entity made with a few components, one destroyed, a
** component attached, set or detached. It keeps what each of its entities
** ought to carry and with what value, and at the next pass looks: at every
** entity through the index and column_base, and at every component through
** its spans. What it says is how many disagreed, which must be none, and a
** digest of its own account, which is the same whatever keeps the rows and
** on however many threads.
**
** Built several times, each with its own SEED and under its own namespace,
** SPACE, so that they run at once.
**
** stage10.md, *What flecs cost*
*/

#include "nstress.h"
#include "native_idle.h"

static const char *const NAMES[KINDS] = {
    SPACE ":K0", SPACE ":K1", SPACE ":K2", SPACE ":K3",
    SPACE ":K4", SPACE ":K5", SPACE ":K6", SPACE ":K7"
};
static const char *const SYSTEMS[KINDS] = {
    "see0", "see1", "see2", "see3", "see4", "see5", "see6", "see7"
};

uint32_t nstress_kinds[KINDS];
uint32_t nstress_seeing[KINDS];

/*
** declare_kind
**
** Declares component `k`, and registers the system that sees it, in the
** phase "check". Their names are not literals, so both are stated without
** the macros that take a literal.
*/
static void declare_kind(const bridge_api_t *api, uint32_t k)
{
    bridge_term_t term[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    uint32_t len = (uint32_t)strlen(NAMES[k]);

    nstress_kinds[k] = api->declare(api, BRIDGE_NATIVE_STATE(api,
        bridge_declaration_t, bridge_native_put(api, NAMES[k], len), len,
        BRIDGE_STRUCT, sizeof(v_t), alignof(v_t),
        bridge_native_put(api, v_t_desc, sizeof(v_t_desc) - 1),
        sizeof(v_t_desc) - 1, BRIDGE_DELETE, BRIDGE_TABLE));
    term[0].slot = nstress_kinds[k];
    nstress_seeing[k] = api->add_system(api, BRIDGE_NATIVE_STATE(api,
        bridge_system_statement_t, bridge_native_put(api, SYSTEMS[k], 4), 4,
        bridge_native_put(api, "check", 5), 5,
        bridge_native_put(api, term, sizeof(term)), 1, 0, 0));
}

/*
** init
**
** Declares the eight components, each with the system that sees it, and
** registers the system that changes them.
*/
BRIDGE_NATIVE(init)
{
    int none = 0;
    uint32_t k = 0;

    while (k < KINDS) {
        declare_kind(api, k);
        k++;
    }
    BRIDGE_NATIVE_ADD_SYSTEM(api, "work", "work", &none, 0, nstress_kinds,
        KINDS);
    return (0);
}
