/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static void permit_in(const config_t *c, uint32_t file,
    const module_key_t *k, names_t *into)
{
    const cfile_t *f = &c->files[file];
    said_t said = {};

    if (config_states_module(f, k, &said)) {
        for (uint32_t i = 0; i < said.value->count; i++)
            names_add(into, said.value->items[i].text);
        return;
    }
    for (uint32_t i = 0; i < f->named_count; i++)
        permit_in(c, f->named[i], k, into);
}

static names_t find_names(const stating_t *s, const char *name)
{
    const char *const names[] = { name };
    const module_key_t k = { { s->of->who.identity, s->of->who.backend,
        "all" }, { names, 1 } };
    const config_t *c = &s->h->config;
    said_t said = {};
    names_t got = {};

    if (c->local >= 0 && config_states_module(&c->files[c->local], &k,
        &said))
        permit_in(c, (uint32_t)c->local, &k, &got);
    else
        permit_in(c, 0, &k, &got);
    return (got);
}

static void say_names(const stating_t *s, const char *key,
    const names_t *got)
{
    if (!s->show)
        return;
    printf("  %s = [", key);
    for (uint32_t i = 0; i < got->count; i++)
        printf("%s\"%s\"", i ? ", " : "", got->names[i]);
    printf("]\n");
}

static void take_losses(bridge_module_config_t *cfg, const value_t *v)
{
    const char **names = v->kind == VALUE_LIST
        ? malloc(sizeof(*names) * (v->count + 1)) : nullptr;

    cfg->loss_count = BRIDGE_NONE;
    if (!names)
        return;
    for (uint32_t i = 0; i < v->count; i++)
        names[i] = v->items[i].text;
    cfg->losses = names;
    cfg->loss_count = v->count;
}

bool state_take_list(const stating_t *s, const module_rule_t *rule,
    bridge_module_config_t *cfg)
{
    names_t got = {};
    const value_t *v = nullptr;

    if (rule->check != CHECK_NAMES) {
        v = state_find_key(s, rule);
        if (v)
            take_losses(cfg, v);
        return (v != nullptr);
    }
    got = find_names(s, rule->name);
    say_names(s, rule->name, &got);
    if (rule->name[0] == 'r')
        cfg->read = got.names;
    else
        cfg->write = got.names;
    if (rule->name[0] == 'r')
        cfg->read_count = got.count;
    else
        cfg->write_count = got.count;
    return (true);
}
