/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "config.h"

static bool is_bare(const char *name)
{
    size_t len = strspn(name, "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-");

    return (len && !name[len]);
}

const char *config_say_key(const char *const *names, uint32_t depth,
    char *out, size_t size)
{
    size_t used = 0;
    const char *form = nullptr;

    out[0] = 0;
    for (uint32_t i = 0; i < depth && used < size; i++) {
        form = is_bare(names[i]) ? "%s%s" : "%s\"%s\"";
        used += (size_t)snprintf(out + used, size - used, form,
            i ? "." : "", names[i]);
    }
    return (out);
}

bool config_say_found(const config_t *c, const char *key, const found_t *f,
    const char *prefix)
{
    char one[256];
    char two[256];

    if (!f->at.value && !f->other.value)
        printf("%snot stated: %s\n", prefix, key);
    if (!f->other.value)
        return (f->at.value != nullptr);
    toml_say(f->at.value, one, sizeof(one));
    toml_say(f->other.value, two, sizeof(two));
    printf("%srefused: %s: %s in %s:%u, %s in %s:%u\n", prefix, key, one,
        c->files[f->at.file].path, f->at.line, two,
        c->files[f->other.file].path, f->other.line);
    return (false);
}

const char *config_say_where(const config_t *c, const said_t *said)
{
    static char where[1100];

    if (said->file == UINT32_MAX)
        return ("its file");
    snprintf(where, sizeof(where), "%s:%u", c->files[said->file].path,
        said->line);
    return (where);
}

void config_say_over(const config_t *c, states_fn *states,
    const void *asked, const said_t *at)
{
    said_t other = {};

    for (uint32_t i = 0; i < c->count; i++)
        if (i != at->file && states(&c->files[i], asked, &other))
            printf(", over %s:%u", c->files[i].path, other.line);
}
