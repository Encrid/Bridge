/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "block.h"

static_assert((int)BRIDGE_LOG_ERR == (int)BRIDGE_ERR);
static_assert((int)BRIDGE_LOG_WARN == (int)BRIDGE_WARN);
static_assert((int)BRIDGE_LOG_INFO == (int)BRIDGE_INFO);
static_assert((int)BRIDGE_LOG_DEBUG == (int)BRIDGE_DEBUG);

static const char OUTSIDE[] =
    "(a log line whose text lies outside the scratch arena)";

static const char *get_writer(const module_t *m,
    const bridge_log_record_t *rec)
{
    if (rec->observer && rec->system < m->observer_count)
        return (m->observers[rec->system]->name);
    if (!rec->observer && rec->system < m->system_count)
        return (m->systems[rec->system]->name);
    return (nullptr);
}

static bool is_in_scratch(const module_t *m, const bridge_log_record_t *rec)
{
    if (rec->text < m->scratch || rec->len > m->scratch_len)
        return (false);
    return (rec->text - m->scratch <= m->scratch_len - rec->len);
}

static void send_line(bridge_t *b, const module_t *m, uint32_t at)
{
    const uint8_t *ring = m->priv + m->ring;
    bridge_log_record_t rec;
    bridge_record_t r = { .text = OUTSIDE, .len = sizeof(OUTSIDE) - 1 };

    memcpy(&rec, ring + (at % m->ring_len) * sizeof(rec), sizeof(rec));
    r.identity = m->identity;
    r.system = get_writer(m, &rec);
    if (is_in_scratch(m, &rec)) {
        r.text = (const char *)m->priv + rec.text;
        r.len = rec.len;
    }
    r.barriers = rec.barriers;
    r.place = rec.place;
    r.arg[0] = rec.arg[0];
    r.arg[1] = rec.arg[1];
    r.level = rec.level;
    say_send(b, &r);
}

void block_drain(bridge_t *b, module_t *m)
{
    bridge_header_t *h = (bridge_header_t *)m->priv;
    uint32_t head = h->ring_head;
    uint32_t written = head - m->ring_tail;
    uint32_t kept = written < m->ring_len ? written : m->ring_len;
    uint32_t i = 0;

    while (i < kept) {
        send_line(b, m, m->ring_tail + i);
        i++;
    }
    if (written > kept)
        say_about(b, &(say_t){ m, nullptr, BRIDGE_WARN }, "%u log lines "
            "were dropped: its ring holds %u", written - kept, m->ring_len);
    m->ring_tail = head;
    h->ring_tail = head;
    h->scratch_used = 0;
}
