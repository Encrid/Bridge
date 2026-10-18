/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "schedule.h"

int schedule_room(bridge_t *b)
{
    uint32_t n = b->max_systems;
    size_t bits = schedule_get_matrix_words(n);
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

static void say_stopped(bridge_t *b, const module_t *m,
    const bridge_system_t *s, const char *fault)
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
        "max_system_failures of %lld, %s", m->identity, most, fault);
}

static void settle_module(bridge_t *b, module_t *m)
{
    const bridge_system_t *s = m->reached;
    const char *fault = m->fault_now;

    if (fault)
        m->failures++;
    else if (m->changed_now && m->failures)
        m->failures--;
    m->changed_now = 0;
    m->fault_now = nullptr;
    m->reached = nullptr;
    if (m->stopped || m->max_failures <= 0)
        return;
    if (!s && m->failures < m->max_failures)
        return;
    m->stopped = 1;
    say_stopped(b, m, s, fault);
    if (m->on_error == BRIDGE_FAIL)
        bridge_stop(b, m->identity, "was stopped");
}

void schedule_account(bridge_t *b)
{
    module_t *m = b->modules;

    while (m) {
        settle_module(b, m);
        m = m->next;
    }
}
