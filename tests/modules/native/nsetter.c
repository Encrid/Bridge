/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nsetter.c -- a native module that sets a component
** in a system: the event a wasm observer subscribes to.
*/

#include "bridge_native.h"
#include "game.h"
#include "native_idle.h"

constexpr uint32_t ENTITIES = 3;

static uint32_t position;
static uint32_t marked;
static uint32_t runs;
static uint64_t ids[ENTITIES];

/*
** mark
**
** Marks the second entity with 77 the first time it runs, and says so. It
** does nothing every other time.
*/
BRIDGE_NATIVE(mark)
{
    marked_t m = { 77 };
    bridge_line_t line = { "marked", 6, 0, 0 };

    runs++;
    if (runs != 1)
        return (0);
    BRIDGE_NATIVE_ATTACH(api, ids[1], marked, m);
    bridge_native_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

/*
** init
**
** Declares the two components and the system, which states that it changes
** the mark, and spawns three entities with a position.
*/
BRIDGE_NATIVE(init)
{
    int none = 0;
    position_t p = { 0.0f, 0.0f };
    uint32_t i = 0;

    position = BRIDGE_NATIVE_COMPONENT(api, "game:Position", position_t,
        BRIDGE_DELETE, BRIDGE_TABLE);
    marked = BRIDGE_NATIVE_COMPONENT(api, "game:Marked", marked_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "mark", "update", &none, 0, &marked, 1);
    while (i < ENTITIES) {
        p.x = (float)i;
        ids[i] = api->spawn(api);
        BRIDGE_NATIVE_ATTACH(api, ids[i], position, p);
        i++;
    }
    return (0);
}
