/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_NATIVE_LOG_H
    #define BRIDGE_NATIVE_LOG_H

    #include "bridge_native_api.h"

typedef struct {
    const char *text;
    uint32_t len;
    uint32_t a0;
    uint32_t a1;
} bridge_line_t;

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
    r->barriers = h->barriers;
    r->place = h->place;
    r->system = h->system;
    r->observer = (uint8_t)h->observer;
    r->level = (uint8_t)level;
    r->text = bridge_native_put(api, line->text, line->len);
    r->len = line->len;
    r->arg[0] = line->a0;
    r->arg[1] = line->a1;
}

#endif
