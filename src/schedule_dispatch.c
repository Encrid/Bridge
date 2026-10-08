/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/schedule_dispatch.c -- one system run: its module told which one it
** is, the call made, and what the call answered kept.
*/

#include <stdio.h>

#include "schedule.h"

/*
** find_slot
**
** Answers the slot of a system among its module's systems, or its
** observers: the number the module was answered when it registered it.
*/
static uint32_t find_slot(const module_t *m, const bridge_system_t *s)
{
    bridge_system_t *const *slots = s->observer ? m->observers : m->systems;
    uint32_t slot = 0;

    while (slots[slot] != s)
        slot++;
    return (slot);
}

/*
** call_system
**
** Calls a system, and marks it failed with why if the call did not return,
** was refused on its way, or answered anything but 0. What is left of the
** work limit only goes down, so it is set again before every call.
**
** limits.md, *What bounds a run*
*/
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

/*
** run_system
**
** Runs one system. Its module's header says which system is running for as
** long as it is, in one store per dispatch: the culprit of anything that
** goes wrong is the system that was running.
**
** validation.md, *What a bad write costs, and what finds it*
*/
static void run_system(module_t *m, bridge_system_t *s)
{
    bridge_header_t *h = (bridge_header_t *)m->priv;
    uint32_t slot = find_slot(m, s);

    h->when = s->when;
    m->running = s;
    m->turn = s->turn;
    h->system = s->observer ? slot | 0x8000u : slot;
    call_system(m, s);
    h->system = UINT32_MAX;
    m->running = nullptr;
}

/*
** schedule_run_job
**
** What a thread runs: a module's systems of one level, in the order it
** registered them.
*/
void schedule_run_job(void *arg)
{
    const schedule_job_t *job = arg;
    uint32_t i = 0;

    while (i < job->count) {
        run_system(job->m, job->list[i]);
        i++;
    }
}
