/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_schema.h"
#include "test_module.h"
#include "idle.h"

#define MARK_FIELDS { uint32_t n; }

BRIDGE_SCHEMA_STRUCT_TYPE(mark_t, MARK_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(mark_t, MARK_FIELDS);

static uint32_t mark;
static uint32_t counting;
static bool stuck;

TEST(count)
{
    const bridge_query_t *q = test_get_query(api, counting);
    bridge_line_t line = { "rows", 4, 0, 0 };
    uint32_t s = 0;

    while (s < q->spans) {
        line.a0 += test_get_span(api, q, s, 1)[1];
        s++;
    }
    test_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

TEST(stick)
{
    if (!stuck)
        api->attach(api, (uint64_t)1 << 32 | TARGET, mark, BRIDGE_ZEROED);
    stuck = true;
    return (0);
}

TEST(init)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    int none = 0;

    mark = TEST_COMPONENT(api, "test:Mark", mark_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    terms[0].slot = mark;
    counting = TEST_ADD_SYSTEM(api, "count", "update", terms, 1, nullptr,
        0, nullptr, 0);
    TEST_ADD_SYSTEM(api, "stick", "update", &none, 0, &mark, 1, nullptr, 0);
    return (0);
}
