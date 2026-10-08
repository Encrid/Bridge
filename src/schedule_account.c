/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/schedule_account.c -- what a dispatch works in, and the failure
** budgets settled at a barrier.
*/

#include <stdlib.h>

#include "schedule.h"

/*
** schedule_room
**
** Makes, once, what a dispatch and a schedule are worked out in, [budget]
** systems wide.
*/
int schedule_room(bridge_t *b)
{
    uint32_t n = b->max_systems ? b->max_systems : 1;
    size_t bits = (size_t)n * ((n + 63) / 64) + 1;
    bool made;

    b->run_jobs = calloc(n, sizeof(schedule_job_t));
    b->run_handed = calloc(n, sizeof(*b->run_handed));
    b->run_lists = calloc(n, sizeof(*b->run_lists));
    b->run_callers = calloc(n, 1);
    b->build_in = calloc(n, sizeof(*b->build_in));
    b->build_bits = calloc(bits, sizeof(*b->build_bits));
    made = b->run_jobs && b->run_handed && b->run_lists && b->run_callers;
    made = made && b->build_in && b->build_bits;
    return (bridge_lacks(b, made ? b : nullptr,
        "what a dispatch and a schedule are worked out in"));
}

/*
** say_stopped
**
** Says that a module is stopped, and by what: the system that reached its
** number, or the module itself.
**
** failure.md, *The budget*
*/
static void say_stopped(bridge_t *b, const module_t *m,
    const bridge_system_t *s)
{
    say_t about = { m, nullptr, BRIDGE_ERR };
    long long most = (long long)m->max_failures;

    if (s) {
        say_about(b, &about, "module '%s' is stopped: its %s '%s' reached "
            "its max_system_failures of %lld", m->identity,
            s->observer ? "observer" : "system", s->name, most);
        return;
    }
    say_about(b, &about, "module '%s' is stopped: it reached its "
        "max_system_failures of %lld, by entities it changed that could "
        "not be placed", m->identity, most);
}

/*
** settle_module
**
** Counts a module's barrier against its budget, and stops it if a system of
** its reached its number or it did itself, by entities it changed that
** could not be placed. `fail` covers a stop as it covers a load: the
** deployment does not run without it.
*/
static void settle_module(bridge_t *b, module_t *m)
{
    const bridge_system_t *s = m->reached;

    if (m->unplaced_now)
        m->failures++;
    else if (m->changed_now && m->failures)
        m->failures--;
    m->changed_now = 0;
    m->unplaced_now = 0;
    m->reached = nullptr;
    if (m->stopped || m->max_failures <= 0)
        return;
    if (!s && m->failures < m->max_failures)
        return;
    m->stopped = 1;
    say_stopped(b, m, s);
    if (m->on_error == BRIDGE_FAIL)
        bridge_stop(b, m->identity, "was stopped");
}

/*
** schedule_account
**
** Stops, at a barrier, each module that reached its failure budget. It is
** counted in invocations and in barriers and never in time, so that the
** same history stops the same module at the same place every run.
*/
void schedule_account(bridge_t *b)
{
    module_t *m = b->modules;

    while (m) {
        settle_module(b, m);
        m = m->next;
    }
}
