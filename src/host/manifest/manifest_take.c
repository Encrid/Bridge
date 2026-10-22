/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "manifest_inside.h"

static int take_number(manifest_t *m, const char *key, const char *value)
{
    char *end = nullptr;
    int64_t n = strtoll(value, &end, 10);

    if (strcmp(key, "abi"))
        return (0);
    if (!*value || *end || n < 0)
        return (-1);
    m->abi = n;
    return (1);
}

static int take_word(manifest_t *m, const char *key, const char *value)
{
    char *word = nullptr;

    if (!strcmp(key, "config"))
        word = m->config;
    if (!strcmp(key, "settings"))
        word = m->settings;
    if (!word)
        return (0);
    if (strlen(value) >= sizeof(m->config))
        return (-1);
    strcpy(word, value);
    return (1);
}

static int take_flag(manifest_t *m, const char *key, const char *value)
{
    bool pinned = !strcmp(key, "pinned");

    if (!pinned && strcmp(key, "reloadable"))
        return (0);
    if (strcmp(value, "true") && strcmp(value, "false"))
        return (-1);
    if (pinned)
        m->pinned = value[0] == 't';
    else
        m->reloadable = value[0] == 't';
    m->has_pinned = m->has_pinned || pinned;
    m->has_reloadable = m->has_reloadable || !pinned;
    return (1);
}

int manifest_take(manifest_t *m, const char *key, const char *value)
{
    int taken = take_number(m, key, value);

    if (!taken)
        taken = take_word(m, key, value);
    if (!taken)
        taken = take_flag(m, key, value);
    return (taken);
}
