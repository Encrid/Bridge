/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/config_say.c -- why a module's settings are refused, said.
*/

#include <stdarg.h>
#include <stdio.h>

#include "config.h"

static const char *const CONFIG_KINDS[] = {
    "unset",
    "a bool",
    "an integer",
    "a float",
    "text"
};

/*
** config_refuse
**
** Writes why the settings are refused, as printf would. Answers 1.
*/
int config_refuse(config_seed_t *seed, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(seed->why, seed->why_len, format, args);
    va_end(args);
    return (1);
}

/*
** config_say_misfit
**
** Refuses a setting whose kind is not one the field it names takes.
**
** config.md, *A module's own settings are a component*
*/
int config_say_misfit(config_seed_t *seed, const bridge_setting_t *s,
    const char *field)
{
    const char *kind = CONFIG_KINDS[0];

    if (s->kind <= BRIDGE_STRING)
        kind = CONFIG_KINDS[s->kind];
    return (config_refuse(seed, "its setting '%.80s' is %s, and the field "
        "it names is %s", s->key, kind, field));
}
