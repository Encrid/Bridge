/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdatomic.h>
#include <string.h>

#include "bridge_inside.h"
#include "say.h"

static bool is_repair(const bridge_t *b, const char *repairing)
{
    size_t n = repairing ? bridge_get_stem_length(repairing) : 0;

    if (!repairing || strlen(b->stopped_stem) != n)
        return (false);
    return (!memcmp(b->stopped_stem, repairing, n));
}

bool bridge_enter(bridge_t *b, bool *owner)
{
    uint64_t me = os_get_thread_id();
    uint64_t was = 0;

    *owner = atomic_compare_exchange_strong_explicit(&b->caller, &was, me,
        memory_order_acquire, memory_order_relaxed);
    return (*owner || was == me);
}

void bridge_leave(bridge_t *b, bool owner)
{
    if (owner)
        atomic_store_explicit(&b->caller, 0, memory_order_release);
}

int bridge_check_entry(bridge_t *b)
{
    const module_t *pinned = bridge_find_pinned(b);

    if (!pinned)
        return (0);
    say_error(b, "refused: module '%s' is pinned to the thread that "
        "loaded it, and this call comes from another", pinned->identity);
    return (1);
}

int bridge_check_call(bridge_t *b, const char *repairing)
{
    if (bridge_check_entry(b))
        return (1);
    if (!b->stopped_by || is_repair(b, repairing))
        return (0);
    say_error(b, "the world has stopped: %s", b->stopped_by);
    return (1);
}
