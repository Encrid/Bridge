/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "os.h"

static const char *const PRESET[] = { "presets" };
static const keyed_t PRESETS = { PRESET, 1 };

static bool is_paths(const value_t *v)
{
    bool paths = v->kind == VALUE_LIST;

    for (uint32_t i = 0; paths && i < v->count; i++)
        paths = v->items[i].kind == VALUE_TEXT;
    return (paths);
}

static bool add_named(config_t *c, int32_t at, uint32_t named)
{
    cfile_t *f = &c->files[at];
    uint32_t *more = realloc(f->named, sizeof(*more) * (f->named_count + 1));

    if (!more)
        return (false);
    f->named = more;
    f->named[f->named_count++] = named;
    return (true);
}

static bool take_named(config_t *c, int32_t at)
{
    said_t said = {};
    const value_t *list = nullptr;
    int32_t got = 0;

    if (!config_states_key(&c->files[at], &PRESETS, &said))
        return (true);
    list = said.value;
    if (!is_paths(list)) {
        printf("refused: %s:%u: presets: a list of files\n",
            c->files[at].path, said.line);
        return (false);
    }
    for (uint32_t i = 0; got >= 0 && i < list->count; i++) {
        got = config_take_file(c, config_path(c, (uint32_t)at,
            list->items[i].text), at, said.line);
        if (got >= 0 && !add_named(c, at, (uint32_t)got))
            got = -1;
    }
    return (got >= 0);
}

int32_t config_take_file(config_t *c, char *path, int32_t by, uint32_t line)
{
    char *whole = path ? os_path_whole(path) : nullptr;
    int32_t at = config_find_whole(c, whole);

    free(whole);
    if (at >= 0 && c->files[at].reading) {
        printf("refused: %s names itself:", c->files[at].path);
        config_say_chain(c, by, at);
        printf(" %s\n", c->files[at].path);
        return (-1);
    }
    if (at >= 0)
        return (at);
    at = path ? config_add_file(c, path, by, line) : -1;
    if (at < 0)
        return (-1);
    c->files[at].reading = true;
    if (!take_named(c, at))
        return (-1);
    c->files[at].reading = false;
    return (at);
}
