/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

static bool take_local(config_t *c, const char *name)
{
    size_t len = strlen(name) + sizeof(".d/local.toml");
    char *path = malloc(len);
    FILE *there = nullptr;

    c->local = -1;
    if (path)
        snprintf(path, len, "%s.d/local.toml", name);
    there = path ? fopen(path, "rb") : nullptr;
    if (!there)
        return (path != nullptr);
    fclose(there);
    c->local = config_add_file(c, path, -1, 0);
    return (c->local >= 0);
}

bool config_read(config_t *c, const char *name)
{
    char *own = malloc(strlen(name) + 1);

    *c = (config_t){ .local = -1 };
    if (own)
        strcpy(own, name);
    if (config_take_file(c, own, -1, 0) != 0)
        return (false);
    return (take_local(c, name));
}
