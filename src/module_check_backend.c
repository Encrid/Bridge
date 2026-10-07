/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/module_check_backend.c -- what a module's config asks of its backend:
** a stated limit demands that the backend can enforce it.
**
** capabilities.md, *Capabilities, so the swap is honest*
*/

#include "module.h"

/*
** check_kept
**
** Refuses a key that states what the backend cannot keep. A guarantee has
** one name, for the key that asks it and the capability that answers.
**
** capabilities.md, *Capabilities, so the swap is honest*
*/
static int check_kept(const module_check_t *k, uint32_t flag, const char *key)
{
    if (k->on->capabilities & flag)
        return (0);
    return (module_refuse(k->b, k->id, "it states %s, which backend '%s' "
        "cannot keep", key, k->on->name));
}

static int check_limits_kept(const module_check_t *k)
{
    const bridge_module_config_t *cfg = k->cfg;

    return ((cfg->work_limit > 0
            && check_kept(k, BACKEND_WORK_LIMIT, "work_limit"))
        || (cfg->memory_limit > 0
            && check_kept(k, BACKEND_MEMORY_LIMIT, "memory_limit"))
        || (cfg->init_timeout > 0
            && check_kept(k, BACKEND_TIMEOUT, "init_timeout"))
        || (cfg->reload_timeout > 0
            && check_kept(k, BACKEND_TIMEOUT, "reload_timeout"))
        || (cfg->shutdown_timeout > 0
            && check_kept(k, BACKEND_TIMEOUT, "shutdown_timeout"))
        || (cfg->hot_reload == BRIDGE_TRUE
            && check_kept(k, BACKEND_RELOAD, "hot_reload")));
}

/*
** module_check_backend
**
** Refuses a config that states what its backend cannot keep. `trusted` asks
** one guarantee, and the backend answers it. The author says "cannot" and
** the deployment "may not", each with a key: a deployment allowing what the
** author says cannot be done is config against config.
**
** wasm.md, *Exports*
*/
int module_check_backend(const module_check_t *k)
{
    const bridge_module_config_t *cfg = k->cfg;
    const open_backend_t *o = k->on;

    if (cfg->trusted == BRIDGE_FALSE && !(o->capabilities & BACKEND_UNTRUSTED))
        return (module_refuse(k->b, k->id, "it is not trusted, and backend "
            "'%s' runs only modules that are", o->name));
    if (check_limits_kept(k))
        return (1);
    if (cfg->hot_reload == BRIDGE_TRUE && cfg->reloadable == BRIDGE_FALSE)
        return (module_refuse(k->b, k->id, "hot_reload is stated, and its "
            "author states it is not reloadable"));
    return (0);
}
