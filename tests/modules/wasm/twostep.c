/* Two systems of one module that conflict in one phase: the order they were registered in is
 * the order they run in. Doubling then adding gives 3 from 1; the other way gives 4. */
#include "bridge-wasm.h"
#include "../game.h"

static uint32_t position, doubling, adding, showing;

static uint32_t first_row(uint32_t query)
{
    uint32_t span = bridge_span(query, 0, 1);
    return BRIDGE_ROW(Position, bridge_base(span, 0), 0, bridge_rows(span));
}

BRIDGE_EXPORT(twice) int twice()
{
    uint32_t at = first_row(doubling);
    BRIDGE_SET(Position, x, at, BRIDGE_GET(Position, x, at) * 2.0f);
    return 0;
}

BRIDGE_EXPORT(more) int more()
{
    uint32_t at = first_row(adding);
    BRIDGE_SET(Position, x, at, BRIDGE_GET(Position, x, at) + 1.0f);
    return 0;
}

BRIDGE_EXPORT(show) int show()
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "x", (uint32_t)BRIDGE_GET(Position, x, first_row(showing)), 0);
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    bridge_term terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    Position p = { 1.0f, 0.0f };
    position = BRIDGE_COMPONENT("game:Position", Position, BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = position;
    doubling = BRIDGE_SYSTEM("twice", "update", terms, 1, nullptr, 0);
    adding = BRIDGE_SYSTEM("more", "update", terms, 1, nullptr, 0);
    terms[0].access = BRIDGE_READ;
    showing = BRIDGE_SYSTEM("show", "present", terms, 1, nullptr, 0);
    BRIDGE_ATTACH(bridge_spawn(), position, p);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
