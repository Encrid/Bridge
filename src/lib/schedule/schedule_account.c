/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "schedule.h"

int schedule_room(bridge_world_t *w)
{
    uint32_t n = w->max_systems;
    size_t bits = schedule_get_matrix_words(n);
    bool made;

    w->run_jobs = calloc(n, sizeof(schedule_job_t));
    w->run_handed = calloc(n, sizeof(*w->run_handed));
    w->run_lists = calloc(n, sizeof(*w->run_lists));
    w->run_callers = calloc(n, 1);
    w->build_in = calloc(n, sizeof(*w->build_in));
    w->build_bits = calloc(bits, sizeof(*w->build_bits));
    made = w->run_jobs && w->run_handed && w->run_lists && w->run_callers;
    made = made && w->build_in && w->build_bits;
    return (bridge_lacks(w, made ? w : nullptr,
        "what a dispatch and a schedule are worked out in"));
}

static void say_stopped(bridge_world_t *w, const module_t *m,
    const bridge_system_t *s, const char *fault)
{
    say_t about = { m, nullptr, BRIDGE_ERR };
    long long most = (long long)m->max_failures;

    if (s) {
        say_about(w, &about, "module '%s' is stopped: its %s '%s' reached "
            "its max_system_failures of %lld", m->identity,
            s->observer ? "observer" : "system", s->name, most);
        return;
    }
    say_about(w, &about, "module '%s' is stopped: it reached its "
        "max_system_failures of %lld, %s", m->identity, most, fault);
}

static void settle_module(bridge_world_t *w, module_t *m)
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
    say_stopped(w, m, s, fault);
    if (m->on_error == BRIDGE_FAIL)
        bridge_stop(w, m->identity, "was stopped");
}

void schedule_account(bridge_world_t *w)
{
    module_t *m = w->modules;

    while (m) {
        settle_module(w, m);
        m = m->next;
    }
}

void schedule_note_end(bridge_world_t *w, const bridge_system_t *s)
{
    say_t about = { s->m, s, BRIDGE_INFO };

    if (!s->ended)
        return;
    w->end_asked = true;
    say_about(w, &about, "%s '%s' asks for the end of the run",
        s->observer ? "observer" : "system", s->name);
}
