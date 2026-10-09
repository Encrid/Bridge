/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/store/store_tables.c -- the build TABLES of
** nstore: fourteen components, and a set of them for each entity that no
** other entity has, so that each entity is in a table of its own.
*/

#include "store.h"

#ifdef TABLES
static const char *const NAMES[KINDS] = {
    "st:T0", "st:T1", "st:T2", "st:T3", "st:T4", "st:T5", "st:T6",
    "st:T7", "st:T8", "st:T9", "st:T10", "st:T11", "st:T12", "st:T13"
};

static uint32_t kinds[KINDS];
static uint32_t seeing;

/*
** attach_kinds
**
** Attaches to entity `i` the kinds whose bits are set in i + 1: no two
** entities carry the same set.
*/
static void attach_kinds(const test_api_t *api, uint32_t i)
{
    uint32_t k = 0;

    while (k < KINDS) {
        if ((i + 1) >> k & 1)
            api->attach(api, nstore_ids[i], kinds[k], BRIDGE_ZEROED);
        k++;
    }
}

/*
** combine
**
** Gives every entity a set of the fourteen kinds that is its own.
*/
TEST(combine)
{
    uint32_t i = 0;

    while (i < COUNT) {
        attach_kinds(api, i);
        i++;
    }
    return (0);
}

/*
** see
**
** Says how many spans a query for the first kind has, and how many rows.
*/
TEST(see)
{
    const bridge_query_t *q = test_get_query(api, seeing);
    uint32_t rows = 0;
    uint32_t s = 0;

    while (s < q->spans) {
        rows += test_get_span(api, q, s, 1)[0];
        s++;
    }
    say(api, "the first kind: spans, rows", q->spans, rows);
    return (0);
}

/*
** declare_kind
**
** Declares kind `k`. Its name is not a literal, so it is stated without the
** macro that takes one.
*/
static void declare_kind(const test_api_t *api, uint32_t k)
{
    uint32_t len = (uint32_t)strlen(NAMES[k]);

    kinds[k] = api->declare(api, TEST_STATE(api,
        bridge_declaration_t, test_put(api, NAMES[k], len), len,
        BRIDGE_STRUCT, sizeof(t_t), alignof(t_t),
        test_put(api, t_t_desc, sizeof(t_t_desc) - 1),
        sizeof(t_t_desc) - 1, BRIDGE_DELETE, BRIDGE_TABLE));
}

/*
** nstore_add_tables
**
** Declares the fourteen components, and registers the system that gives
** each entity a set of them of its own and the system that sees the first.
*/
void nstore_add_tables(const test_api_t *api)
{
    int none = 0;
    bridge_term_t first[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    uint32_t k = 0;

    while (k < KINDS) {
        declare_kind(api, k);
        k++;
    }
    first[0].slot = kinds[0];
    TEST_ADD_SYSTEM(api, "combine", "combine", &none, 0, kinds, 14);
    seeing = TEST_ADD_SYSTEM(api, "see", "look", first, 1, nullptr,
        0);
}

#endif
