/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"
#include "os.h"

static bool add_path(char ***paths, uint32_t *count, char *path)
{
    char *whole = path ? os_path_whole(path) : nullptr;
    char *other = nullptr;
    char **more = nullptr;
    bool same = false;

    for (uint32_t i = 0; whole && !same && i < *count; i++) {
        other = os_path_whole((*paths)[i]);
        same = other && !strcmp(other, whole);
        free(other);
    }
    free(whole);
    if (!same && path)
        more = realloc(*paths, sizeof(*more) * (*count + 1));
    if (more)
        *paths = more;
    if (more)
        (*paths)[(*count)++] = path;
    return (same || more);
}

static uint32_t collect(const host_t *h, const char *field, char ***paths)
{
    const char *const names[] = { "content", field };
    const keyed_t k = { names, 2 };
    said_t said = {};
    uint32_t count = 0;

    *paths = nullptr;
    for (uint32_t f = 0; f < h->config.count; f++) {
        if (!config_states_key(&h->config.files[f], &k, &said))
            continue;
        for (uint32_t i = 0; i < said.value->count; i++)
            (void)add_path(paths, &count, config_path(&h->config, f,
                said.value->items[i].text));
    }
    return (count);
}

bool config_content(host_t *h)
{
    h->path_count = collect(h, "paths", &h->paths);
    h->ignore_count = collect(h, "ignore", &h->ignore);
    if (!h->path_count)
        printf("not stated: content.paths\n");
    return (h->path_count > 0);
}

int config_load(const host_t *h, const char *identity, const char *prefix)
{
    const char *const own[] = { "content", "load", identity };
    const char *const all[] = { "content", "load", "all" };
    keyed_t k = { own, 3 };
    found_t f = config_find(&h->config, config_states_key, &k);

    if (!f.at.value) {
        k.names = all;
        f = config_find(&h->config, config_states_key, &k);
    }
    if (!config_say_found(&h->config, "content.load.all", &f, prefix))
        return (-1);
    return (f.at.value->flag ? 1 : 0);
}
