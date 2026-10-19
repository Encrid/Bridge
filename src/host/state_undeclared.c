/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static bool has_declared(const names_t *declared, const char *name)
{
    for (uint32_t i = 0; i < declared->count; i++)
        if (!strcmp(declared->names[i], name))
            return (true);
    return (false);
}

static uint32_t say_undeclared_in(const stating_t *s, const cfile_t *f,
    const names_t *declared)
{
    const char *const any[] = { s->of->who.identity, nullptr };
    const entry_t *e = nullptr;
    uint32_t count = 0;

    for (uint32_t i = 0; i < f->toml.count; i++) {
        e = &f->toml.entries[i];
        if (!state_is_setting(e, any) || has_declared(declared, e->key[3]))
            continue;
        printf("%sit takes no config.%s, stated in %s:%u\n", s->prefix,
            e->key[3], f->path, e->line);
        count++;
    }
    return (count);
}

uint32_t state_undeclared(const stating_t *s, const names_t *declared)
{
    uint32_t count = 0;

    for (uint32_t i = 0; i < s->h->config.count; i++)
        count += say_undeclared_in(s, &s->h->config.files[i], declared);
    return (count);
}
