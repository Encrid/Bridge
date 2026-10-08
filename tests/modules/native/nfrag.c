/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nfrag.c -- what it costs a system when the entities
** it reads are spread over many tables: one component every entity carries,
** and markers that split them into SPLIT sets of the same size. The system
** reads the one component of every row and says how many rows there were,
** so that a host can time it and know the loop ran.
**
** Built once for each SPLIT: 1, 16, 256, 4096. The entities are the same
** number every time. Twelve markers make 2^12 sets at the most.
**
** stage10.md, *The repair*
*/

#include "bridge_native.h"
#include "native/native_measured.h"
#include "native_idle.h"
#include "nfrag.h"

constexpr uint32_t COUNT = 204'800;
constexpr uint32_t MARKERS = 12;
static const char *const NAMES[MARKERS] = {
    "st:M0", "st:M1", "st:M2", "st:M3", "st:M4", "st:M5",
    "st:M6", "st:M7", "st:M8", "st:M9", "st:M10", "st:M11"
};

static uint32_t value;
static uint32_t markers[MARKERS];
static uint32_t reading;

static void attach_markers(const bridge_api_t *api, uint64_t id, uint32_t set)
{
    uint32_t k = 0;

    while (k < MARKERS) {
        if (set >> k & 1)
            api->attach(api, id, markers[k], BRIDGE_ZEROED);
        k++;
    }
}

/*
** fill
**
** Spawns the entities. Each is made with all it will carry, the value and
** the markers of its set, so each is placed once.
*/
BRIDGE_NATIVE(fill)
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

/*
** sum
**
** Reads the value of every row, and says once a call what was read and
** over how many spans.
*/
NATIVE_MEASURED BRIDGE_NATIVE(sum)
{
    const bridge_query_t *q = bridge_native_get_query(api, reading);
    bridge_line_t line = { "rows, spans", 11, 0, q->spans };
    const uint32_t *span;
    const v_t *column;
    uint32_t s = 0;
    uint32_t r;

    while (s < q->spans) {
        span = bridge_native_get_span(api, q, s, 1);
        column = (const v_t *)(api->shared_base + span[2]);
        r = 0;
        while (r < span[0]) {
            line.a0 += column[r].v;
            r++;
        }
        line.a0 += span[0];
        s++;
    }
    bridge_native_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

/*
** declare_marker
**
** Declares one marker by its name, as a struct, or as a tag in a build with
** TAGS, and answers its slot.
*/
static uint32_t declare_marker(const bridge_api_t *api, const char *name)
{
    uint32_t len = (uint32_t)strlen(name);
    uint32_t at = bridge_native_put(api, name, len);

#ifdef TAGS
    return (api->declare(api, BRIDGE_NATIVE_STATE(api, bridge_declaration_t,
        at, len, BRIDGE_TAG, 0, 1, 0, 0, BRIDGE_DELETE, BRIDGE_TABLE)));
#else
    return (api->declare(api, BRIDGE_NATIVE_STATE(api, bridge_declaration_t,
        at, len, BRIDGE_STRUCT, sizeof(m_t), alignof(m_t),
        bridge_native_put(api, m_t_desc, sizeof(m_t_desc) - 1),
        sizeof(m_t_desc) - 1, BRIDGE_DELETE, BRIDGE_TABLE)));
#endif
}

/*
** init
**
** Declares the value and the markers, and the two systems: `fill` states
** that it attaches the value and every marker.
*/
BRIDGE_NATIVE(init)
{
    int none = 0;
    bridge_term_t one[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    uint32_t filled[1 + MARKERS];
    uint32_t k = 0;

    value = BRIDGE_NATIVE_COMPONENT(api, "st:V", v_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    filled[0] = value;
    while (k < MARKERS) {
        markers[k] = declare_marker(api, NAMES[k]);
        filled[1 + k] = markers[k];
        k++;
    }
    one[0].slot = value;
    BRIDGE_NATIVE_ADD_SYSTEM(api, "fill", "spawn", &none, 0, filled,
        1 + MARKERS);
    reading = BRIDGE_NATIVE_ADD_SYSTEM(api, "sum", "look", one, 1, nullptr,
        0);
    return (0);
}
