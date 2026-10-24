/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "say.h"
#include "schedule.h"

static bool may_write(const bridge_system_t *s, const storage_type_t *type)
{
    uint32_t j = 0;

    while (j < s->terms) {
        if (s->term[j].touches && s->term[j].touches->type == type
            && s->term[j].writes)
            return (true);
        j++;
    }
    return (false);
}

static void on_changed(void *ctx, const storage_type_t *type)
{
    bridge_system_t *s = ctx;
    const say_t about = { s->m, s, BRIDGE_WARN };

    if (may_write(s, type))
        return;
    say_about(s->m->w, &about, "system '%s' wrote a column that it did not "
        "declare to write", s->name);
}

void schedule_validate_system(bridge_system_t *s)
{
    storage_validate_scan(s->m->w->storage, on_changed, s);
}
