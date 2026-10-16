/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "config.h"

static uint32_t get_text_len(const bridge_setting_t *s)
{
    if (s->kind != BRIDGE_STRING || !s->v.s)
        return (0);
    return ((uint32_t)strlen(s->v.s));
}

static uint32_t put_text(config_seed_t *seed, const char *text, uint32_t len)
{
    uint32_t at = seed->m->text_at + seed->text_used;

    memcpy(seed->b->shared + at, text, len);
    seed->text_used += (len + 7u) & ~7u;
    return (at);
}

int config_seed_text(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s)
{
    uint32_t len = get_text_len(s);
    bridge_text_t text = { 0, len };

    if (s->kind != BRIDGE_STRING)
        return (config_say_misfit(seed, s, "a struct, which only text into "
            "a bridge_text_t fills"));
    text.at = put_text(seed, s->v.s, len);
    memcpy(seed->row + field->offset, &text, sizeof(text));
    return (0);
}

int config_seed_struct(config_seed_t *seed,
    [[maybe_unused]] const schema_field_t *field, const bridge_setting_t *s)
{
    return (config_say_misfit(seed, s, "a struct, which only text into a "
        "bridge_text_t fills"));
}

int config_seed_array(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s)
{
    uint32_t len = get_text_len(s);

    if (field->kind != SCHEMA_CHAR || s->kind != BRIDGE_STRING)
        return (config_say_misfit(seed, s, "an array, which only text into "
            "a char array fills"));
    if (len + 1 > field->count)
        return (config_refuse(seed, "its setting '%.80s' is %u bytes of "
            "text, and the field it names holds %d with the zero that ends "
            "it", s->key, len, (int)field->count));
    memcpy(seed->row + field->offset, s->v.s, len);
    return (0);
}
