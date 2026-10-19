/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

const value_t *state_find_key(const stating_t *s, const module_rule_t *rule)
{
    const char *const names[] = { rule->name };
    const module_key_t k = { { s->of->who.identity, s->of->who.backend,
        "all" }, { names, 1 } };
    const shown_t how = { "  ", config_states_module, &k };
    found_t f = config_find(&s->h->config, config_states_module, &k);

    if (!f.at.value && !f.other.value)
        f.at = (said_t){ state_find_own(s, rule), UINT32_MAX, 0 };
    if (!config_say_found(&s->h->config, rule->name, &f, s->prefix))
        return (nullptr);
    if (s->show)
        config_say_value(&s->h->config, rule->name, &f, &how);
    return (f.at.value);
}

static void put_value(bridge_module_config_t *cfg, const module_rule_t *rule,
    const value_t *v)
{
    int64_t n = v->kind == VALUE_WHOLE ? v->whole : rule->none;
    uint8_t one = (uint8_t)n;
    uint32_t four = (uint32_t)n;

    if (rule->check == CHECK_FLAG)
        one = v->flag ? BRIDGE_TRUE : BRIDGE_FALSE;
    if (rule->check == CHECK_ERROR)
        one = v->text[0] == 'f' ? BRIDGE_FAIL : BRIDGE_SKIP;
    if (rule->width == 1)
        memcpy((char *)cfg + rule->at, &one, 1);
    if (rule->width == 4)
        memcpy((char *)cfg + rule->at, &four, 4);
    if (rule->width == 8)
        memcpy((char *)cfg + rule->at, &n, 8);
}

bool state_take_key(const stating_t *s, const module_rule_t *rule,
    bridge_module_config_t *cfg)
{
    const value_t *v = nullptr;

    if (rule->check == CHECK_AUTHOR)
        return (true);
    if (!rule->width)
        return (state_take_list(s, rule, cfg));
    v = state_find_key(s, rule);
    if (v)
        put_value(cfg, rule, v);
    return (v != nullptr);
}
