/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "offsets_idle.h"

BRIDGE_EXPORT(shout) int shout(void)
{
    constexpr uint32_t size = sizeof(bridge_log_record_t);
    uint32_t head = BRIDGE_HEADER(ring_head);
    uint32_t at = BRIDGE_HEADER(ring) + head % BRIDGE_HEADER(ring_len) * size;

    acc_own_st(offsetof(bridge_header_t, ring_head), head + 1u);
    acc_own_st(at, BRIDGE_HEADER(barriers));
    acc_own_st(at + 4u, BRIDGE_HEADER(place));
    acc_own_st(at + 8u, BRIDGE_HEADER(system));
    acc_own_st(at + 12u, 200u << 8 | (BRIDGE_HEADER(observer) & 0xffu));
    acc_own_st(at + 16u, bridge_put("over every severity", 19));
    acc_own_st(at + 20u, 19u);
    acc_own_st(at + 24u, 0u);
    acc_own_st(at + 28u, 0u);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("shout", "update", &none, 0, nullptr, 0);
    return (0);
}
