/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdarg.h>
#include <stdio.h>

#include "config.h"

static const char *const CONFIG_KINDS[] = {
    "a bool",
    "an integer",
    "a float",
    "text"
};

int config_refuse(config_seed_t *seed, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(seed->why, seed->why_len, format, args);
    va_end(args);
    return (1);
}

int config_say_misfit(config_seed_t *seed, const bridge_setting_t *s,
    const char *field)
{
    return (config_refuse(seed, "its setting '%.80s' is %s, and the field "
        "it names is %s", s->key, CONFIG_KINDS[s->kind - 1], field));
}
