/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "bridge_inside.h"
#include "say.h"

size_t bridge_get_stem_length(const char *identity)
{
    const char *at = strchr(identity, '@');

    if (!at)
        return (strlen(identity));
    return ((size_t)(at - identity));
}

const module_t *bridge_find_pinned(const bridge_world_t *w)
{
    const module_t *m = w->modules;
    uint64_t thread = os_get_thread_id();

    while (m && (!m->pinned || m->thread == thread))
        m = m->next;
    return (m);
}

void bridge_stop(bridge_world_t *w, const char *identity, const char *what)
{
    int stem = (int)bridge_get_stem_length(identity);

    if (w->stopped_by)
        return;
    snprintf(w->stopped_why, sizeof(w->stopped_why),
        "module '%s' %s and its on_error is fail", identity, what);
    snprintf(w->stopped_name, sizeof(w->stopped_name), "%.*s", stem,
        identity);
    w->stopped_by = w->stopped_why;
    w->stopped_stem = w->stopped_name;
    say_error(w, "the world has stopped: %s", w->stopped_why);
}

void bridge_resume(bridge_world_t *w, const module_t *m)
{
    say_t about = { m, nullptr, BRIDGE_INFO };

    if (!w->stopped_stem)
        return;
    w->stopped_by = nullptr;
    w->stopped_stem = nullptr;
    say_about(w, &about, "the world runs again: module '%s' is back",
        m->identity);
}

void bridge_give_up(bridge_world_t *w, const module_t *m)
{
    say_t about = { m, nullptr, BRIDGE_INFO };

    if (!w->stopped_stem)
        return;
    w->stopped_by = nullptr;
    w->stopped_stem = nullptr;
    say_about(w, &about, "the world runs again: module '%s' is unloaded, "
        "and it goes on without it", m->identity);
}
