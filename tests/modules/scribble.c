/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "reach.h"
#include "game.h"

#define OTHER_FIELDS { uint64_t n; }
BRIDGE_SCHEMA_STRUCT_TYPE(other_t, OTHER_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(other_t, OTHER_FIELDS);

static uint32_t a;
static uint32_t b;
static uint64_t id;
static uint64_t extra;
static int done;

static void *cell(const test_api_t *api, uint32_t slot, uint32_t size)
{
    const bridge_header_t *h = test_get_header(api);
    const bridge_entity_t *e = &get_entity_index(api)[(uint32_t)id];

    return (api->shared_base
        + get_column_bases(api)[e->table * h->types + slot] + e->row * size);
}

TEST(scribble)
{
    ((position_t *)cell(api, a, sizeof(position_t)))->x = 7.0f;
    ((marked_t *)cell(api, b, sizeof(marked_t)))->by = 9;
    if (!done)
        api->destroy(api, extra);
    done = 1;
    return (0);
}

TEST(init)
{
    bridge_term_t terms[3] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 },
        { 0, BRIDGE_READ, BRIDGE_AND, 0 }, { 0, BRIDGE_READ, BRIDGE_NOT, 0 }};
    position_t p = { 1.0f, 2.0f };
    marked_t k = { 3 };
    uint32_t c;

    a = TEST_COMPONENT(api, "game:a", position_t, BRIDGE_STRIP, BRIDGE_TABLE);
    b = TEST_COMPONENT(api, "game:b", marked_t, BRIDGE_STRIP, BRIDGE_TABLE);
    c = TEST_COMPONENT(api, "game:c", other_t, BRIDGE_STRIP, BRIDGE_TABLE);
    terms[0].slot = a;
    terms[1].slot = b;
    terms[2].slot = c;
    TEST_ADD_SYSTEM(api, "scribble", "update", terms, 3, nullptr, 0,
        nullptr, 0);
    id = api->spawn(api);
    TEST_ATTACH(api, id, a, p);
    TEST_ATTACH(api, id, b, k);
    extra = api->spawn(api);
    TEST_ATTACH(api, extra, a, p);
    return (0);
}

TEST(reload)
{
    return (0);
}

TEST(shutdown)
{
    return (0);
}
