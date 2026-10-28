/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "measured.h"
#include "idle.h"
#include "frag.h"
#include "reach.h"

constexpr uint32_t COUNT = 204'800;
constexpr uint32_t MARKERS = 12;
static const char *const NAMES[MARKERS] = {
    "st:M0", "st:M1", "st:M2", "st:M3", "st:M4", "st:M5",
    "st:M6", "st:M7", "st:M8", "st:M9", "st:M10", "st:M11"
};

static uint32_t value;
static uint32_t markers[MARKERS];
static uint32_t reading;

static void attach_markers(const test_api_t *api, uint64_t id, uint32_t set)
{
    uint32_t k = 0;

    while (k < MARKERS) {
        if (set >> k & 1)
            api->attach(api, id, markers[k], BRIDGE_ZEROED);
        k++;
    }
}

TEST(fill)
{
    uint64_t id;
    uint32_t i = 0;

    while (i < COUNT) {
        id = api->spawn(api);
        api->attach(api, id, value, BRIDGE_ZEROED);
        attach_markers(api, id, i % SPLIT);
        i++;
    }
    return (0);
}

TEST_MEASURED TEST(sum)
{
    const bridge_query_t *q = test_get_query(api, reading);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    bridge_line_t line = { "rows, spans", 11, 0, q->spans };
    const uint32_t *span;
    const v_t *column;
    uint32_t s = 0;
    uint32_t r;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        column = (const v_t *)(api->shared_base
            + bases[span[0] * types + value]);
        r = 0;
        while (r < span[1]) {
            line.a0 += column[r].v;
            r++;
        }
        line.a0 += span[1];
        s++;
    }
    test_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

static uint32_t declare_marker(const test_api_t *api, const char *name)
{
    uint32_t len = (uint32_t)strlen(name);
    uint32_t at = test_put(api, name, len);

#ifdef TAGS
    return (api->declare(api, TEST_STATE(api, bridge_declaration_t,
        at, len, BRIDGE_TAG, 0, 1, 0, 0, BRIDGE_DELETE, BRIDGE_TABLE,
        BRIDGE_ON_DESTROY_REMOVE)));
#else
    return (api->declare(api, TEST_STATE(api, bridge_declaration_t,
        at, len, BRIDGE_STRUCT, sizeof(m_t), alignof(m_t),
        test_put(api, m_t_desc, sizeof(m_t_desc) - 1),
        sizeof(m_t_desc) - 1, BRIDGE_DELETE, BRIDGE_TABLE,
        BRIDGE_ON_DESTROY_REMOVE)));
#endif
}

TEST(init)
{
    int none = 0;
    bridge_term_t one[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    uint32_t filled[1 + MARKERS];
    uint32_t k = 0;

    value = TEST_COMPONENT(api, "st:V", v_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    filled[0] = value;
    while (k < MARKERS) {
        markers[k] = declare_marker(api, NAMES[k]);
        filled[1 + k] = markers[k];
        k++;
    }
    one[0].slot = value;
    TEST_ADD_SYSTEM(api, "fill", "spawn", &none, 0, filled,
        1 + MARKERS, nullptr, 0);
    reading = TEST_ADD_SYSTEM(api, "sum", "look", one, 1, nullptr,
        0, nullptr, 0);
    return (0);
}
