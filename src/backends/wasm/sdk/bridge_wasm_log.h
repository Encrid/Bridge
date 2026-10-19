/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_WASM_LOG_H
    #define BRIDGE_WASM_LOG_H

    #include "bridge_wasm_scratch.h"

    #define BRIDGE_LOG(level, text, a0, a1) \
        bridge_log(level, &(bridge_line_t){ text, sizeof(text) - 1, a0, a1 })

typedef struct {
    const char *text;
    uint32_t len;
    uint32_t a0;
    uint32_t a1;
} bridge_line_t;

static inline void bridge_log(uint32_t level, const bridge_line_t *line)
{
    constexpr uint32_t size = sizeof(bridge_log_record_t);
    uint32_t head;
    uint32_t at;

    if (level > BRIDGE_HEADER(log_level))
        return;
    head = BRIDGE_HEADER(ring_head);
    acc_own_st(offsetof(bridge_header_t, ring_head), head + 1u);
    if (head - BRIDGE_HEADER(ring_tail) >= BRIDGE_HEADER(ring_len))
        return;
    at = BRIDGE_HEADER(ring) + (head % BRIDGE_HEADER(ring_len)) * size;
    acc_own_st(at, BRIDGE_HEADER(barriers));
    acc_own_st(at + 4u, BRIDGE_HEADER(place));
    acc_own_st(at + 8u, BRIDGE_HEADER(system));
    acc_own_st(at + 12u,
        (BRIDGE_HEADER(observer) & 0xffu) | (level & 0xffu) << 8);
    acc_own_st(at + 16u, bridge_put(line->text, line->len));
    acc_own_st(at + 20u, line->len);
    acc_own_st(at + 24u, line->a0);
    acc_own_st(at + 28u, line->a1);
}

#endif
