/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** backends/native/sdk/bridge_native_log.h -- the log: stores, and no call.
**
** What is compiled with this file is a work based on Bridge and is under
** the same licence. Other terms: docs/licence.md.
**
** log.md, *The log*
*/

#ifndef BRIDGE_NATIVE_LOG_H
    #define BRIDGE_NATIVE_LOG_H

    #include "bridge_native_api.h"

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
** bridge_native_log
**
** Writes a line into the module's ring, if its level is one the module
** logs. A ring that is full drops the line, and it is counted by the head
** having moved.
**
** log.md, *The log*
*/
static inline void bridge_native_log(const bridge_api_t *api, uint32_t level,
    const bridge_line_t *line)
{
    bridge_header_t *h = bridge_native_get_header(api);
    bridge_log_record_t *r = (bridge_log_record_t *)(api->private_base
        + h->ring);
    uint32_t head = h->ring_head;

    if (level > h->log_level)
        return;
    h->ring_head = head + 1u;
    if (head - h->ring_tail >= h->ring_len)
        return;
    r += head % h->ring_len;
    r->when = h->when;
    r->pad = 0;
    r->level = (uint8_t)level;
    r->system = (uint16_t)h->system;
    r->text = bridge_native_put(api, line->text, line->len);
    r->len = line->len;
    r->arg[0] = line->a0;
    r->arg[1] = line->a1;
}

#endif
