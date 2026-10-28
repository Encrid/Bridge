/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "husk.h"

uint32_t husk_body;
uint32_t husk_handle;
uint32_t husk_finding;
uint32_t husk_on_body;
uint32_t husk_on_handle;
uint32_t husk_on_lone;

static void declare_both(void)
{
    husk_body = BRIDGE_COMPONENT("husk:Body", body_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    husk_handle = BRIDGE_DECLARE_AS(BRIDGE_STRUCT, "husk:Handle", handle_t,
        BRIDGE_STRIP, BRIDGE_TABLE, ON_DESTROY);
}

static void register_all(void)
{
    bridge_term_t one[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 },
        { 0, BRIDGE_READ, BRIDGE_NOT, 0 }};
    uint32_t both[2] = { husk_body, husk_handle };
    int none = 0;

    BRIDGE_ADD_SYSTEM("step", "update", &none, 0, both, 2, nullptr, 0);
    one[0].slot = husk_handle;
    husk_finding = BRIDGE_ADD_SYSTEM("look", "present", one, 1, nullptr, 0,
        nullptr, 0);
    husk_on_handle = BRIDGE_OBSERVER("heard_handle",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr,
        0, nullptr, 0);
    one[0].slot = husk_body;
    husk_on_body = BRIDGE_OBSERVER("heard_body",
        BRIDGE_ON_ENTER | BRIDGE_ON_LEAVE, one, 1, nullptr, 0, nullptr, 0);
    two[0].slot = husk_handle;
    two[1].slot = husk_body;
    husk_on_lone = BRIDGE_OBSERVER("heard_lone", BRIDGE_ON_ENTER, two, 2,
        nullptr, 0, nullptr, 0);
}

BRIDGE_EXPORT(init) int init(void)
{
    declare_both();
#ifndef TWIN
    register_all();
#endif
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
    declare_both();
    register_all();
    husk_frame = HUSK_DONE;
    return (0);
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    uint32_t count = 0;
    uint32_t sum = 0;
    uint32_t value;
    uint32_t i = 0;

    while (i < 3) {
        value = husk_get_value(husk_ids[i], husk_handle);
        count += value != 0;
        sum += value;
        i++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "release", count, sum);
    return (0);
}
