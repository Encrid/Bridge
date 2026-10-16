/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "module.h"

kept_text_t *config_find_kept(bridge_t *b, const char *stem)
{
    uint32_t i = b->kept_count;

    while (i && strcmp(b->kept[i - 1].stem, stem))
        i--;
    if (!i)
        return (nullptr);
    return (&b->kept[i - 1]);
}

static kept_text_t *add_kept(bridge_t *b, const module_t *m)
{
    size_t room = (b->kept_count + 1) * sizeof(*b->kept);
    kept_text_t *more = realloc(b->kept, room);
    char *named = copy_text(m->stem, strlen(m->stem));
    range_t *owned = malloc(m->own_count * sizeof(*owned));

    if (more)
        b->kept = more;
    if (!more || !named || !owned) {
        free(named);
        free(owned);
        (void)bridge_lacks(b, nullptr, "a share that stays: the arena "
            "loses its bytes");
        return (nullptr);
    }
    memcpy(owned, m->owned, m->own_count * sizeof(*owned));
    b->kept[b->kept_count] = (kept_text_t){ named, 0, 0, owned,
        m->own_count };
    b->kept_count++;
    return (&b->kept[b->kept_count - 1]);
}

void config_keep(bridge_t *b, module_t *m)
{
    kept_text_t *kept = add_kept(b, m);

    if (!kept)
        return;
    kept->at = m->text_at;
    kept->room = m->text_room;
}

static bool is_kept_held(const bridge_t *b, const kept_text_t *kept)
{
    for (uint32_t i = 0; i < b->type_count; i++) {
        if (module_count_pointing(b, kept->owned, kept->own_count,
            b->types[i]))
            return (true);
    }
    return (false);
}

void config_give_back_free(bridge_t *b)
{
    kept_text_t *kept;
    uint32_t i = 0;

    while (i < b->kept_count) {
        kept = &b->kept[i];
        if (is_kept_held(b, kept)) {
            i++;
        } else {
            module_give_share(b, kept->owned, kept->own_count);
            free(kept->stem);
            free(kept->owned);
            b->kept_count--;
            *kept = b->kept[b->kept_count];
        }
    }
}
