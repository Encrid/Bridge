/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "share.h"

#ifdef HOLD
typedef shared_t Shared;
BRIDGE_SCHEMA_STRUCT_TYPE(box_t, { Shared s; });
BRIDGE_SCHEMA_STRUCT_DESC(box_t, { Shared s; });
#endif

static uint32_t shared;
static uint32_t doing;

BRIDGE_EXPORT(run) int run(void)
{
    uint32_t spans = bridge_get_span_count(doing);
    uint32_t rows = 0;
    uint32_t sum = 0;
    uint32_t table;
    uint32_t count;
    uint32_t span;
    uint32_t s = 0;
    uint32_t r;

    while (s < spans) {
        span = bridge_get_span(doing, s);
        table = bridge_get_span_table(span);
        count = bridge_get_rows(span);
        r = 0;
        while (r < count) {
            sum += BRIDGE_GET(shared_t, x, shared, table, r);
            r++;
        }
        rows += count;
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "saw", rows, sum);
    return (0);
}

static int declare(void)
{
#ifdef WRITER
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
#else
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
#endif

    shared = BRIDGE_COMPONENT("game:Shared", shared_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    (void)BRIDGE_DECLARE_TAG("game:Keep", BRIDGE_STRIP, BRIDGE_TABLE);
#ifdef HOLD
    (void)BRIDGE_COMPONENT("game:Box", box_t, BRIDGE_DELETE, BRIDGE_TABLE);
#endif
    terms[0].slot = shared;
#ifdef WRITER
    doing = BRIDGE_ADD_SYSTEM("run", "update", terms, 1, nullptr, 0,
        nullptr, 0);
#else
    doing = BRIDGE_ADD_SYSTEM("run", "present", terms, 1, nullptr, 0,
        nullptr, 0);
#endif
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    shared_t v = {};

    if (declare())
        return (1);
    v.x = 42;
    BRIDGE_ATTACH(bridge_spawn(), shared, v);
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
#ifdef TRAP
    return (1);
#else
    return (declare());
#endif
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
#ifdef SAY
    BRIDGE_LOG(BRIDGE_LOG_INFO, "shutdown", WIDE, 0);
#endif
    return (0);
}
