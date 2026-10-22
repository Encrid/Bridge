/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static void add_phase_names(const host_t *h, const char *table,
    names_t *into)
{
    const char *const names[] = { "module", table, "phases" };
    const keyed_t under = { names, 3 };
    names_t found = config_names(&h->config, &under);

    for (uint32_t i = 0; i < found.count; i++)
        names_add(into, found.names[i]);
    free(found.names);
}

static bool take_phase(const stating_t *s, const char *name,
    bridge_phase_name_t *into)
{
    const char *const names[] = { "phases", name };
    const module_key_t k = { { s->of->who.identity, s->of->who.backend,
        "all" }, { names, 2 } };
    const shown_t how = { "  ", config_states_module, &k };
    found_t f = config_find(&s->h->config, config_states_module, &k);
    char key[256];

    snprintf(key, sizeof(key), "phases.%s", name);
    if (!config_say_found(&s->h->config, key, &f, s->prefix))
        return (false);
    if (s->show)
        config_say_value(&s->h->config, key, &f, &how);
    *into = (bridge_phase_name_t){ name, f.at.value->text };
    return (true);
}

bool state_take_phases(const stating_t *s, bridge_module_config_t *cfg)
{
    names_t names = {};
    bridge_phase_name_t *map = nullptr;
    bool taken = true;

    add_phase_names(s->h, s->of->who.identity, &names);
    add_phase_names(s->h, s->of->who.backend, &names);
    add_phase_names(s->h, "all", &names);
    map = names.count ? malloc(sizeof(*map) * names.count) : nullptr;
    for (uint32_t i = 0; map && i < names.count; i++)
        taken = take_phase(s, names.names[i], &map[i]) && taken;
    cfg->phases = map;
    cfg->phase_count = map ? names.count : 0;
    return (taken);
}
