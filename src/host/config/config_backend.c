/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static char *with_suffix(char *named)
{
    size_t len = named ? strlen(named) : 0;
    size_t end = strlen(LIBRARY_SUFFIX);
    char *whole = nullptr;

    if (!named || (len >= end && !strcmp(named + len - end, LIBRARY_SUFFIX)))
        return (named);
    whole = malloc(len + end + 1);
    if (whole)
        snprintf(whole, len + end + 1, "%s%s", named, LIBRARY_SUFFIX);
    return (whole);
}

static enum arrive get_arrive(const value_t *v)
{
    static const char *const WORDS[] = { "", "ask", "load", "leave" };

    for (uint32_t i = 1; v && i < 4; i++)
        if (!strcmp(v->text, WORDS[i]))
            return ((enum arrive)i);
    return (ARRIVE_UNSTATED);
}

static bool take_backend(host_t *h, const char *name, uint32_t at)
{
    const char *const library[] = { "backend", name };
    const char *const arrive[] = { "content", "arrive", name };
    said_t lib = config_find_key(&h->config, library, 2);
    said_t way = config_find_key(&h->config, arrive, 3);
    char *path = lib.value ? config_path(&h->config, lib.file,
        lib.value->text) : nullptr;

    h->held[at] = (held_t){ name, with_suffix(path), get_arrive(way.value) };
    h->backends[at] = (bridge_backend_t){ .name = name,
        .library = h->held[at].library };
    return (h->held[at].library && way.value);
}

bool config_backends(host_t *h)
{
    static const char *const BACKEND[] = { "backend" };
    const keyed_t under = { BACKEND, 1 };
    names_t names = config_names(&h->config, &under);
    bool taken = names.count > 0;

    if (!names.count)
        printf("not stated: backend\n");
    h->held = calloc(names.count + 1, sizeof(*h->held));
    h->backends = calloc(names.count + 1, sizeof(*h->backends));
    for (uint32_t i = 0; h->held && h->backends && i < names.count; i++)
        taken = take_backend(h, names.names[i], i) && taken;
    h->backend_count = names.count;
    return (taken && h->held && h->backends);
}

const held_t *get_held(const host_t *h, const char *name)
{
    for (uint32_t i = 0; i < h->backend_count; i++)
        if (!strcmp(h->held[i].name, name))
            return (&h->held[i]);
    return (nullptr);
}
