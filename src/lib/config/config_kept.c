/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "module.h"

kept_text_t *config_find_kept(bridge_world_t *w, const char *stem)
{
    uint32_t i = w->kept_count;

    while (i && strcmp(w->kept[i - 1].stem, stem))
        i--;
    if (!i)
        return (nullptr);
    return (&w->kept[i - 1]);
}

static kept_text_t *add_kept(bridge_world_t *w, const module_t *m)
{
    size_t room = (w->kept_count + 1) * sizeof(*w->kept);
    kept_text_t *more = realloc(w->kept, room);
    char *named = copy_text(m->stem, strlen(m->stem));
    range_t *owned = malloc(m->own_count * sizeof(*owned));

    if (more)
        w->kept = more;
    if (!more || !named || !owned) {
        free(named);
        free(owned);
        (void)bridge_lacks(w, nullptr, "a share that stays: the arena "
            "loses its bytes");
        return (nullptr);
    }
    memcpy(owned, m->owned, m->own_count * sizeof(*owned));
    w->kept[w->kept_count] = (kept_text_t){ named, 0, 0, owned,
        m->own_count };
    w->kept_count++;
    return (&w->kept[w->kept_count - 1]);
}

void config_keep(bridge_world_t *w, module_t *m)
{
    kept_text_t *kept = add_kept(w, m);

    if (!kept)
        return;
    kept->at = m->text_at;
    kept->room = m->text_room;
}

static bool is_kept_held(const bridge_world_t *w, const kept_text_t *kept)
{
    for (uint32_t i = 0; i < w->type_count; i++) {
        if (module_count_pointing(w, kept->owned, kept->own_count,
            w->types[i]))
            return (true);
    }
    return (false);
}

void config_give_back_free(bridge_world_t *w)
{
    kept_text_t *kept;
    uint32_t i = 0;

    while (i < w->kept_count) {
        kept = &w->kept[i];
        if (is_kept_held(w, kept)) {
            i++;
        } else {
            module_give_share(w, kept->owned, kept->own_count);
            free(kept->stem);
            free(kept->owned);
            w->kept_count--;
            *kept = w->kept[w->kept_count];
        }
    }
}
