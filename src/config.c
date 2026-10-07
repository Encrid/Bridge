/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/config.c -- a module's own settings, written into the component its
** config names.
*/

#include <stdlib.h>
#include <string.h>

#include "config.h"

/*
** check_none
**
** Refuses settings handed to a module that states it has no config
** component.
**
** config.md, *A module's own settings are a component*
*/
static int check_none(config_seed_t *seed)
{
    if (!seed->cfg->setting_count)
        return (0);
    return (config_refuse(seed, "it is handed %u settings, and states that "
        "it has no config component", seed->cfg->setting_count));
}

/*
** check_taken
**
** Refuses a config component that is already another module's settings: it
** is one value, so one module's.
**
** config.md, *A module's own settings are a component*
*/
static int check_taken(config_seed_t *seed)
{
    const module_t *other = seed->b->modules;

    while (other && other->config != seed->c)
        other = other->next;
    if (!other)
        return (0);
    return (config_refuse(seed, "its config is '%.100s', which is already "
        "the settings of module '%.80s'", seed->c->name, other->identity));
}

/*
** find_config
**
** Finds the component the module's config names, among what its init
** declared: a struct, and a singleton, that no other module's settings are.
**
** config.md, *A module's own settings are a component*
*/
static int find_config(config_seed_t *seed)
{
    const module_t *m = seed->m;
    const char *name = seed->cfg->config;
    declared_type_t *c = nullptr;
    uint32_t i = 0;

    while (i < m->declared_count) {
        if (!strcmp(m->declared[i]->name, name))
            c = m->declared[i];
        i++;
    }
    if (!c || c->kind != BRIDGE_STRUCT)
        return (config_refuse(seed, "its config is '%.100s', which is not a "
            "struct its init declared", name));
    if (c->storage != BRIDGE_SINGLETON)
        return (config_refuse(seed, "its config is '%.100s', which is not "
            "declared a singleton", c->name));
    seed->c = c;
    return (check_taken(seed));
}

/*
** seed_all
**
** Writes every setting into one value of the config component, and gives the
** singleton that value when none was refused.
*/
static int seed_all(config_seed_t *seed)
{
    const bridge_module_config_t *cfg = seed->cfg;
    declared_type_t *c = seed->c;
    int refused = 0;
    uint32_t i = 0;

    seed->row = calloc(1, c->size);
    while (!refused && i < cfg->setting_count) {
        refused = config_seed_setting(seed, &cfg->settings[i]);
        i++;
    }
    if (!refused) {
        storage_give_single(seed->b->storage, c->type, seed->row, c->size);
        seed->m->config = c;
    }
    free(seed->row);
    return (refused);
}

/*
** config_seed
**
** Writes a module's settings into the component its config names.
*/
int config_seed(bridge_t *b, module_t *m, const bridge_module_config_t *cfg,
    backend_err_t *err)
{
    config_seed_t seed = { .b = b, .m = m, .cfg = cfg, .why = err->text };

    seed.why_len = sizeof(err->text);
    if (!strcmp(cfg->config, BRIDGE_NO_CONFIG))
        return (check_none(&seed));
    if (find_config(&seed) || config_place_text(&seed))
        return (1);
    return (seed_all(&seed));
}
