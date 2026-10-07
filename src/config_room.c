/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/config_room.c -- where the text of a module's settings goes, known
** before any of it is written.
*/

#include <string.h>

#include "config.h"

/*
** get_text_size
**
** Answers how many bytes of the arena one setting takes: its text rounded up
** to eight, when it is a text for a field that is a bridge_text_t, and 0 for
** any other.
*/
static uint32_t get_text_size(const config_seed_t *seed,
    const bridge_setting_t *s)
{
    const schema_t *of = &seed->c->s;
    uint32_t k = 0;

    if (!s->key || s->kind != BRIDGE_STRING || !s->v.s)
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

/*
** reuse_kept
**
** Takes the block this module's text went in the last time it was loaded,
** when `total` bytes still fit there.
*/
static void reuse_kept(config_seed_t *seed, uint32_t total)
{
    module_t *m = seed->m;
    const kept_text_t *kept = config_find_kept(seed->b, m->stem);

    if (!kept || kept->room < total)
        return;
    m->text_at = kept->at;
    m->text_room = kept->room;
}

/*
** config_place_text
**
** Finds where the text of the settings goes: the block it had, or as much of
** the module's persistent share.
**
** config.md, *A module's own settings are a component*
*/
int config_place_text(config_seed_t *seed)
{
    module_t *m = seed->m;
    uint32_t total = count_text(seed);

    if (!total)
        return (0);
    reuse_kept(seed, total);
    if (m->text_room)
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
