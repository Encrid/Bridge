/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "os.h"
#include "schedule.h"

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
    s->ended = status == BRIDGE_END;
    if (!status || s->ended)
        return;
    snprintf(s->why, sizeof(s->why), "it answered %d", status);
    s->failed = 1;
}

static void after_call(module_t *m, bridge_system_t *s, uint64_t start)
{
    if (m->w->profile) {
        s->nanos += os_get_now() - start;
        s->calls += s->observer;
    }
    if (m->w->validate)
        schedule_validate_system(s);
}

static void run_system(module_t *m, bridge_system_t *s)
{
    bridge_header_t *h = (bridge_header_t *)m->priv;
    uint64_t start = 0;

    h->barriers = m->w->barriers;
    h->place = s->place;
    m->running = s;
    m->turn = s->turn;
    h->system = s->slot;
    h->observer = s->observer ? 1 : 0;
    if (m->w->profile)
        start = os_get_now();
    call_system(m, s);
    after_call(m, s, start);
    h->system = UINT32_MAX;
    h->observer = 0;
    m->running = nullptr;
}

void schedule_run_job(void *arg)
{
    const schedule_job_t *job = arg;
    uint32_t i = 0;

    while (i < job->count) {
        if (schedule_is_due(job->list[i]))
            run_system(job->m, job->list[i]);
        i++;
    }
}
