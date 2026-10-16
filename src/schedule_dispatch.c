/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "schedule.h"

static uint32_t find_slot(const module_t *m, const bridge_system_t *s)
{
    bridge_system_t *const *slots = s->observer ? m->observers : m->systems;
    uint32_t slot = 0;

    while (slots[slot] != s)
        slot++;
    return (slot);
}

static void call_system(module_t *m, bridge_system_t *s)
{
    backend_err_t err = {};
    int32_t status = 0;
    uint64_t limit = (uint64_t)m->work_limit;

    if (m->work_limit > 0)
        m->on->api.instance_set_work_limit(m->instance, limit);
    m->refused = 0;
    if (m->on->api.call(s->fn, &status, &err) || m->refused) {
        snprintf(s->why, sizeof(s->why), "%s", m->refused ? m->why : err.text);
        s->failed = 1;
        return;
    }
    if (!status)
        return;
    snprintf(s->why, sizeof(s->why), "it answered %d", status);
    s->failed = 1;
}

static void run_system(module_t *m, bridge_system_t *s)
{
    bridge_header_t *h = (bridge_header_t *)m->priv;
    uint32_t slot = find_slot(m, s);

    h->barriers = m->b->barriers;
    h->place = s->place;
    m->running = s;
    m->turn = s->turn;
    h->system = slot;
    h->observer = s->observer ? 1 : 0;
    call_system(m, s);
    h->system = UINT32_MAX;
    h->observer = 0;
    m->running = nullptr;
}

void schedule_run_job(void *arg)
{
    const schedule_job_t *job = arg;
    uint32_t i = 0;

    while (i < job->count) {
        run_system(job->m, job->list[i]);
        i++;
    }
}
