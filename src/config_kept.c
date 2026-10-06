/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/config_kept.c -- where an unloaded module's settings text is,
** remembered for its next load.
*/

#include <stdlib.h>
#include <string.h>

#include "config.h"

/*
** config_find_kept
**
** Answers what is remembered of the settings text of the module `stem`, or
** nullptr.
*/
kept_text_t *config_find_kept(bridge_t *b, const char *stem)
{
    uint32_t i = 0;

    while (i < b->kept_count && strcmp(b->kept[i].stem, stem))
        i++;
    if (i == b->kept_count)
        return (nullptr);
    return (&b->kept[i]);
}

/*
** add_kept
**
** Makes room to remember one more module's text. nullptr if there is no
** memory for it: an unload does not fail, what is lost is the reuse of that
** range by the module's next load, and it is said.
**
** failure.md, *Out of memory*
*/
static kept_text_t *add_kept(bridge_t *b, const char *stem)
{
    size_t room = (b->kept_count + 1) * sizeof(*b->kept);
    kept_text_t *more = realloc(b->kept, room);
    char *named = copy(stem, strlen(stem));

    if (more)
        b->kept = more;
    if (!more || !named) {
        free(named);
        (void)bridge_short(b, nullptr, "the place of an unloaded module's "
            "settings text, which its next load will not write over");
        return (nullptr);
    }
    b->kept[b->kept_count].stem = named;
    b->kept_count++;
    return (&b->kept[b->kept_count - 1]);
}

/*
** config_keep
**
** Remembers where an unloaded module's settings text is.
*/
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

/*
** config_forget
**
** Forgets where a module's settings text was.
*/
void config_forget(bridge_t *b, const module_t *m)
{
    kept_text_t *kept = config_find_kept(b, m->stem);

    if (!kept)
        return;
    free(kept->stem);
    b->kept_count--;
    *kept = b->kept[b->kept_count];
}
