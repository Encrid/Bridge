/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static void say_files(const config_t *c)
{
    const cfile_t *f = nullptr;

    for (uint32_t i = 0; i < c->count; i++) {
        f = &c->files[i];
        printf("file %s", f->path);
        if (f->by >= 0)
            printf(", named by %s:%u", c->files[f->by].path, f->by_line);
        printf("%s\n", (int32_t)i == c->local ? ", the local file" : "");
    }
}

static void say_key(const config_t *c, const char *const *names,
    uint32_t depth)
{
    const keyed_t k = { names, depth };
    const shown_t how = { "", config_states_key, &k };
    found_t f = config_find(c, config_states_key, &k);
    char key[256];

    config_say_value(c, config_say_key(names, depth, key, sizeof(key)), &f,
        &how);
}

static void say_under(const config_t *c, const char *top, const char *next)
{
    const char *const under[] = { top, next };
    const keyed_t k = { under, next ? 2 : 1 };
    names_t names = config_names(c, &k);
    const char *key[3] = { top, next, nullptr };

    for (uint32_t i = 0; i < names.count; i++) {
        key[k.depth] = names.names[i];
        say_key(c, key, k.depth + 1);
    }
    free(names.names);
}

static void say_world(const host_t *h)
{
    static const char *const ONE[] = { "phases", "threads", "rounds", "live" };

    for (size_t i = 0; i < sizeof(ONE) / sizeof(*ONE); i++)
        say_key(&h->config, &ONE[i], 1);
    say_under(&h->config, "budget", nullptr);
    say_under(&h->config, "backend", nullptr);
    say_under(&h->config, "content", "arrive");
    say_under(&h->config, "content", "load");
    for (uint32_t i = 0; i < h->path_count; i++)
        printf("content.paths: %s\n", h->paths[i]);
    for (uint32_t i = 0; i < h->ignore_count; i++)
        printf("content.ignore: %s\n", h->ignore[i]);
    for (uint32_t i = 0; i < h->order_count; i++)
        printf("order: %s before %s\n", h->order[i].before,
            h->order[i].after);
}

int show_config(host_t *h, bool taken)
{
    say_files(&h->config);
    if (!taken)
        return (1);
    say_world(h);
    if (walk_content(h, show_module, h) < 0)
        return (1);
    return (h->counts.failed ? 1 : 0);
}
