/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static void put_setting(const stating_t *s, const said_t *said,
    bridge_setting_t *set)
{
    const value_t *v = said->value;
    bool path = said->file != UINT32_MAX
        && state_is_path(&s->h->config, said);

    if (v->kind == VALUE_TEXT)
        *set = (bridge_setting_t){ .key = set->key, .kind = BRIDGE_STRING,
            .v.s = path ? config_path(&s->h->config, said->file, v->text)
            : v->text };
    if (v->kind == VALUE_WHOLE)
        *set = (bridge_setting_t){ .key = set->key, .kind = BRIDGE_INT,
            .v.i = v->whole };
    if (v->kind == VALUE_REAL)
        *set = (bridge_setting_t){ .key = set->key, .kind = BRIDGE_FLOAT,
            .v.f = v->real };
    if (v->kind == VALUE_FLAG)
        *set = (bridge_setting_t){ .key = set->key, .kind = BRIDGE_BOOL,
            .v.b = v->flag };
}

static bool take_setting(const stating_t *s, const char *name,
    bridge_setting_t *set)
{
    const char *const two[] = { s->of->who.identity, name };
    const shown_t how = { "  ", state_states_setting, two };
    found_t f = config_find(&s->h->config, state_states_setting, two);
    char key[160];

    snprintf(key, sizeof(key), "config.%s", name);
    if (!f.at.value && !f.other.value)
        f.at = state_own_setting(s, name);
    if (!config_say_found(&s->h->config, key, &f, s->prefix))
        return (false);
    if (s->show)
        config_say_value(&s->h->config, key, &f, &how);
    set->key = name;
    put_setting(s, &f.at, set);
    return (true);
}

static names_t split_names(const char *settings)
{
    char *copy = malloc(strlen(settings) + 1);
    char *name = nullptr;
    names_t got = {};

    if (copy)
        strcpy(copy, settings);
    name = copy ? strtok(copy, " ") : nullptr;
    while (name) {
        names_add(&got, name);
        name = strtok(nullptr, " ");
    }
    return (got);
}

bool state_settings(const stating_t *s, bridge_module_config_t *cfg)
{
    names_t declared = split_names(s->of->who.settings);
    bridge_setting_t *set = calloc(declared.count + 1, sizeof(*set));
    bool taken = !state_undeclared(s, &declared) && set;

    if (declared.count && !strcmp(s->of->who.config, BRIDGE_NO_CONFIG)) {
        printf("%sit names no component, and settings\n", s->prefix);
        taken = false;
    }
    for (uint32_t i = 0; set && i < declared.count; i++)
        taken = take_setting(s, declared.names[i], &set[i]) && taken;
    cfg->settings = declared.count ? set : nullptr;
    cfg->setting_count = declared.count;
    return (taken);
}
