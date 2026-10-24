/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "offsets_idle.h"

static void put_line(uint32_t observer, uint32_t system, uint32_t text,
    uint32_t len)
{
    constexpr uint32_t size = sizeof(bridge_log_record_t);
    uint32_t head = BRIDGE_HEADER(ring_head);
    uint32_t at = BRIDGE_HEADER(ring) + head % BRIDGE_HEADER(ring_len) * size;

    acc_own_st(offsetof(bridge_header_t, ring_head), head + 1u);
    acc_own_st(at, BRIDGE_HEADER(barriers));
    acc_own_st(at + 4u, BRIDGE_HEADER(place));
    acc_own_st(at + 8u, system);
    acc_own_st(at + 12u, 200u << 8 | observer);
    acc_own_st(at + 16u, text);
    acc_own_st(at + 20u, len);
    acc_own_st(at + 24u, 0u);
    acc_own_st(at + 28u, 0u);
}

BRIDGE_EXPORT(shout) int shout(void)
{
#ifdef FORGED
    put_line(1u, 5u, 0u, 4u);
    put_line(0u, BRIDGE_HEADER(system), BRIDGE_HEADER(scratch),
        BRIDGE_HEADER(scratch_len) + 1u);
#else
    put_line(BRIDGE_HEADER(observer) & 0xffu, BRIDGE_HEADER(system),
        bridge_put("over every severity", 19), 19u);
#endif
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("shout", "update", &none, 0, nullptr, 0, nullptr, 0);
    return (0);
}
