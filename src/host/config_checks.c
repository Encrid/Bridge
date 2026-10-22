/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "host.h"

void take_checks(host_t *h)
{
    static const char *const profile[] = { "checks", "profile" };
    static const char *const validate[] = { "checks", "validate" };
    const value_t *v = config_find_key(&h->config, profile, 2).value;

    if (v)
        h->profile = v->flag;
    v = config_find_key(&h->config, validate, 2).value;
    if (v)
        h->validate = v->flag;
}

const char *config_check_checks(const entry_t *e, bool local)
{
    if (e->depth != 2 || (strcmp(e->key[1], "profile")
        && strcmp(e->key[1], "validate")))
        return ("no key of Bridge-Host's or Bridge's");
    if (local)
        return ("fixed when the world is made: not in the local file");
    return (config_check_value(&e->value, CHECK_FLAG));
}
