/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "config.h"

kept_text_t *config_find_kept(bridge_t *b, const char *stem)
{
    uint32_t i = 0;

    while (i < b->kept_count && strcmp(b->kept[i].stem, stem))
        i++;
    if (i == b->kept_count)
        return (nullptr);
    return (&b->kept[i]);
}

static kept_text_t *add_kept(bridge_t *b, const char *stem)
{
    size_t room = (b->kept_count + 1) * sizeof(*b->kept);
    kept_text_t *more = realloc(b->kept, room);
    char *named = copy_text(stem, strlen(stem));

    if (more)
        b->kept = more;
    if (!more || !named) {
        free(named);
        (void)bridge_lacks(b, nullptr, "the place of an unloaded module's "
            "settings text, which its next load will not write over");
        return (nullptr);
    }
    b->kept[b->kept_count].stem = named;
    b->kept_count++;
    return (&b->kept[b->kept_count - 1]);
}

void config_keep(bridge_t *b, module_t *m)
{
    kept_text_t *kept = config_find_kept(b, m->stem);

    if (!m->text_room)
        return;
    if (!kept)
        kept = add_kept(b, m->stem);
    if (!kept)
        return;
    kept->at = m->text_at;
    kept->room = m->text_room;
}

void config_forget(bridge_t *b, const module_t *m)
{
    kept_text_t *kept = config_find_kept(b, m->stem);

    if (!kept)
        return;
    free(kept->stem);
    b->kept_count--;
    *kept = b->kept[b->kept_count];
}
