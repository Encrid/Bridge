/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/block_log.c -- a module's log ring, drained to the sink.
**
** A module's four names for a level and a host's are two seams' names for
** the same numbers, each header being whole without the other: they are
** held equal here, where one becomes the other.
*/

#include <string.h>

#include "block.h"

static_assert((int)BRIDGE_LOG_ERR == (int)BRIDGE_ERR);
static_assert((int)BRIDGE_LOG_WARN == (int)BRIDGE_WARN);
static_assert((int)BRIDGE_LOG_INFO == (int)BRIDGE_INFO);
static_assert((int)BRIDGE_LOG_DEBUG == (int)BRIDGE_DEBUG);

static const char OUTSIDE[] =
    "(a log line whose text lies outside the scratch arena)";

/*
** get_writer
**
** Answers the name of the system or the observer a record says wrote it, by
** its slot, or nullptr if the module has none there.
*/
static const char *get_writer(const module_t *m, uint32_t system)
{
    uint32_t observer = system - 0x8000u;

    if (system < m->system_count)
        return (m->systems[system]->name);
    if (system >= 0x8000u && observer < m->observer_count)
        return (m->observers[observer]->name);
    return (nullptr);
}

static bool is_in_scratch(const module_t *m, const bridge_log_record_t *rec)
{
    if (rec->text < m->scratch || rec->len > m->scratch_len)
        return (false);
    return (rec->text - m->scratch <= m->scratch_len - rec->len);
}

/*
** send_line
**
** Hands the sink the record at `at` of a module's ring. It is copied out
** first, and its text is checked to lie in the scratch arena, since a
** module can write all of its private memory.
*/
static void send_line(bridge_t *b, const module_t *m, uint32_t at)
{
    const uint8_t *ring = m->priv + m->ring;
    bridge_log_record_t rec;
    bridge_record_t r = { .text = OUTSIDE, .len = sizeof(OUTSIDE) - 1 };

    memcpy(&rec, ring + (at % m->ring_len) * sizeof(rec), sizeof(rec));
    r.identity = m->identity;
    r.system = get_writer(m, rec.system);
    if (is_in_scratch(m, &rec)) {
        r.text = (const char *)m->priv + rec.text;
        r.len = rec.len;
    }
    r.when = rec.when;
    r.arg[0] = rec.arg[0];
    r.arg[1] = rec.arg[1];
    r.level = rec.level;
    b->sink(b->ud, &r);
}

/*
** block_drain
**
** Hands a module's log ring to the sink: the lines it holds, and how many
** more were written than it holds. Then the scratch arena is the module's
** again, after the drain, since a line's text lives there.
*/
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
