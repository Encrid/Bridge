/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "husk.h"

uint32_t husk_heard[3];

static uint32_t count_rows(uint32_t query)
{
    uint32_t spans = bridge_get_span_count(query);
    uint32_t rows = 0;
    uint32_t s = 0;

    while (s < spans) {
        rows += bridge_get_rows(bridge_get_span(query, s));
        s++;
    }
    return (rows);
}

uint32_t husk_get_value(uint64_t id, uint32_t slot)
{
    uint64_t e = bridge_get_location(id);

    if (!bridge_is_alive(id)
        || !bridge_has_column(slot, bridge_get_entity_table(e)))
        return (0);
    return (acc_get_32(slot, bridge_get_entity_table(e),
        bridge_get_entity_row(e), 0));
}

static void say_first(uint32_t frame)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "e alive", frame,
        (uint32_t)bridge_is_alive(husk_ids[0]));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "e kept", frame,
        husk_get_value(husk_ids[0], husk_handle));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "e body", frame,
        husk_get_value(husk_ids[0], husk_body));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "g kept", frame,
        husk_get_value(husk_ids[2], husk_handle));
}

BRIDGE_EXPORT(look) int look(void)
{
    uint32_t found = count_rows(husk_finding);
    uint32_t frame = husk_frame;

    if (frame > HUSK_DONE) {
        BRIDGE_LOG(BRIDGE_LOG_INFO, "found again", frame - HUSK_DONE, found);
        return (0);
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "found", frame, found);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "heard body", frame, husk_heard[0]);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "heard handle", frame, husk_heard[1]);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "heard lone", frame, husk_heard[2]);
    say_first(frame);
    memset(husk_heard, 0, sizeof(husk_heard));
    return (0);
}
