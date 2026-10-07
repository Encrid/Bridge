/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** backends/wasm/sdk/bridge_wasm_log.h -- the log: stores, and no call.
**
** log.md, *The log*
*/

#ifndef BRIDGE_WASM_LOG_H
    #define BRIDGE_WASM_LOG_H

    #include "bridge_wasm_scratch.h"

/*
** A line of the log whose text is a literal, with two numbers.
**
** log.md, *The log*
*/
    #define BRIDGE_LOG(level, text, a0, a1) \
        bridge_log(level, &(bridge_line_t){ text, sizeof(text) - 1, a0, a1 })

/*
** What a line of the log holds: its text, how long that is, and two
** numbers.
**
** log.md, *The log*
*/
typedef struct {
    const char *text;
    uint32_t len;
    uint32_t a0;
    uint32_t a1;
} bridge_line_t;

/*
** bridge_log
**
** Writes a line into the module's ring, if its level is one the module
** logs. A ring that is full drops the line, and it is counted by the head
** having moved.
**
** log.md, *The log*
*/
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
    acc_own_st(at, BRIDGE_HEADER(when));
    acc_own_st(at + 4u, (level & 0xffu) << 8 | BRIDGE_HEADER(system) << 16);
    acc_own_st(at + 8u, bridge_put(line->text, line->len));
    acc_own_st(at + 12u, line->len);
    acc_own_st(at + 16u, line->a0);
    acc_own_st(at + 20u, line->a1);
}

#endif
