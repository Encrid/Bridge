/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdatomic.h>
#include <string.h>

#include "bridge_inside.h"
#include "say.h"

static bool is_repair(const bridge_world_t *w, const char *repairing)
{
    size_t n = repairing ? bridge_get_stem_length(repairing) : 0;

    if (!repairing || strlen(w->stopped_stem) != n)
        return (false);
    return (!memcmp(w->stopped_stem, repairing, n));
}

bool bridge_enter(bridge_world_t *w, bool *owner)
{
    uint64_t me = os_get_thread_id();
    uint64_t was = 0;

    *owner = atomic_compare_exchange_strong_explicit(&w->caller, &was, me,
        memory_order_acquire, memory_order_relaxed);
    return (*owner || was == me);
}

void bridge_leave(bridge_world_t *w, bool owner)
{
    if (owner)
        atomic_store_explicit(&w->caller, 0, memory_order_release);
}

int bridge_check_entry(bridge_world_t *w)
{
    const module_t *pinned = bridge_find_pinned(w);

    if (!pinned)
        return (0);
    say_error(w, "refused: module '%s' is pinned to the thread that "
        "loaded it, and this call comes from another", pinned->identity);
    return (1);
}

int bridge_check_call(bridge_world_t *w, const char *repairing)
{
    if (bridge_check_entry(w))
        return (1);
    if (!w->stopped_by || is_repair(w, repairing))
        return (0);
    say_error(w, "the world has stopped: %s", w->stopped_by);
    return (1);
}
