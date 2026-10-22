/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static bool is_found(const host_t *h, const char *identity)
{
    for (int i = 0; i < FILES; i++)
        if (h->files[i].of && !strcmp(h->files[i].of->who.identity, identity))
            return (true);
    return (false);
}

static const char *get_named(const entry_t *e)
{
    if (e->depth >= 2 && !strcmp(e->key[0], "module")
        && strchr(e->key[1], '@'))
        return (e->key[1]);
    if (e->depth == 3 && !strcmp(e->key[0], "content")
        && !strcmp(e->key[1], "load") && strchr(e->key[2], '@'))
        return (e->key[2]);
    return (nullptr);
}

static bool say_unfound(const host_t *h, const cfile_t *f,
    const entry_t *e, names_t *said)
{
    const char *named = get_named(e);
    uint32_t count = said->count;

    if (!named || is_found(h, named))
        return (false);
    names_add(said, named);
    if (said->count == count)
        return (false);
    printf("refused: %s:%u: %s: no module found is it\n", f->path, e->line,
        named);
    return (true);
}

static void say_found_ones(const host_t *h)
{
    printf("found:");
    for (int i = 0; i < FILES; i++)
        if (h->files[i].of && h->files[i].of->who.identity[0])
            printf(" %s", h->files[i].of->who.identity);
    printf("\n");
}

uint32_t config_unfound(const host_t *h)
{
    names_t said = {};
    uint32_t refused = 0;
    const cfile_t *f = nullptr;

    for (uint32_t i = 0; i < h->config.count; i++) {
        f = &h->config.files[i];
        for (uint32_t j = 0; j < f->toml.count; j++)
            refused += say_unfound(h, f, &f->toml.entries[j], &said);
    }
    free(said.names);
    if (refused)
        say_found_ones(h);
    return (refused);
}
