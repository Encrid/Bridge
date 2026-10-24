/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_inside.h"

static int profile_rows(bridge_world_t *w, bridge_profile_row_t *out, int max)
{
    const bridge_system_t *s;
    int count = 0;

    while (count < (int)w->system_count) {
        s = w->systems[count];
        if (out && count < max) {
            out[count].name = s->full;
            out[count].calls = s->calls;
            out[count].nanos = s->nanos;
        }
        count++;
    }
    return (count);
}

int bridge_profile(bridge_world_t *w, bridge_profile_row_t *out, int max)
{
    bool owner;
    int result;

    if (!bridge_enter(w, &owner))
        return (-1);
    result = profile_rows(w, out, max);
    bridge_leave(w, owner);
    return (result);
}
