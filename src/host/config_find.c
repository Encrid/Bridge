/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "config.h"

static found_t find_in(const config_t *c, uint32_t file, states_fn *states,
    const void *asked)
{
    const cfile_t *f = &c->files[file];
    found_t got = {};
    found_t peer = {};

    if (states(f, asked, &got.at)) {
        got.at.file = file;
        return (got);
    }
    for (uint32_t i = 0; i < f->named_count; i++) {
        peer = find_in(c, f->named[i], states, asked);
        if (peer.other.value)
            return (peer);
        if (got.at.value && peer.at.value
            && !toml_is_same(got.at.value, peer.at.value))
            return ((found_t){ got.at, peer.at });
        if (!got.at.value)
            got = peer;
    }
    return (got);
}

found_t config_find(const config_t *c, states_fn *states, const void *asked)
{
    found_t got = {};

    if (c->local >= 0 && states(&c->files[c->local], asked, &got.at)) {
        got.at.file = (uint32_t)c->local;
        return (got);
    }
    return (find_in(c, 0, states, asked));
}

said_t config_find_key(const config_t *c, const char *const *names,
    uint32_t depth)
{
    const keyed_t k = { names, depth };
    found_t f = config_find(c, config_states_key, &k);
    char key[256];

    config_say_key(names, depth, key, sizeof(key));
    if (!config_say_found(c, key, &f, ""))
        return ((said_t){});
    return (f.at);
}

void config_say_value(const config_t *c, const char *key,
    const found_t *f, const shown_t *how)
{
    char value[512];

    if (!f->at.value)
        return;
    toml_say(f->at.value, value, sizeof(value));
    printf("%s%s = %s  %s", how->prefix, key, value,
        config_say_where(c, &f->at));
    if (how->states && f->at.file != UINT32_MAX)
        config_say_over(c, how->states, how->asked, &f->at);
    printf("\n");
}
