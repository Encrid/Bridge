/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "config.h"

static uint32_t get_text_size(const config_seed_t *seed,
    const bridge_setting_t *s)
{
    const schema_t *of = &seed->c->s;
    uint32_t k = 0;

    if (s->kind != BRIDGE_STRING)
        return (0);
    while (k < of->field_count && strcmp(of->fields[k].name, s->key))
        k++;
    if (k == of->field_count || of->fields[k].kind != SCHEMA_TEXT)
        return (0);
    if (of->fields[k].count != 1)
        return (0);
    return (((uint32_t)strlen(s->v.s) + 7u) & ~7u);
}

static uint32_t count_text(const config_seed_t *seed)
{
    uint32_t total = 0;
    uint32_t i = 0;

    while (i < seed->cfg->setting_count) {
        total += get_text_size(seed, &seed->cfg->settings[i]);
        i++;
    }
    return (total);
}

static bool reuse_kept(config_seed_t *seed, uint32_t total)
{
    module_t *m = seed->m;
    const kept_text_t *kept = config_find_kept(seed->w, m->stem);

    if (!kept || kept->room < total)
        return (false);
    m->text_at = kept->at;
    m->text_room = kept->room;
    return (true);
}

int config_place_text(config_seed_t *seed)
{
    module_t *m = seed->m;
    uint32_t total = count_text(seed);

    if (!total)
        return (0);
    if (reuse_kept(seed, total))
        return (0);
    if (m->persistent <= 0 || total > m->arena_left)
        return (config_refuse(seed, "its settings hold %u bytes of text, "
            "and its persistent share has %u left", total,
            m->persistent > 0 ? m->arena_left : 0));
    m->text_at = m->arena_at;
    m->text_room = total;
    m->arena_at += total;
    m->arena_left -= total;
    return (0);
}

void config_keep_text(const config_seed_t *seed)
{
    memcpy(seed->w->shared + seed->m->text_at, seed->row + seed->c->size,
        seed->text_used);
}
