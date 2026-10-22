/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static const char *const BEFORE[] = { "order", "before" };
static const keyed_t ORDERED = { BEFORE, 2 };

static bool is_pair(const char *a, const char *b, const char *const *two)
{
    return ((!strcmp(a, two[0]) && !strcmp(b, two[1]))
        || (!strcmp(a, two[1]) && !strcmp(b, two[0])));
}

static bool states_pair(const cfile_t *f, const void *asked, said_t *said)
{
    const char *const *two = asked;
    said_t list = {};
    const value_t *pair = nullptr;

    if (!config_states_key(f, &ORDERED, &list))
        return (false);
    for (uint32_t i = 0; i < list.value->count; i++) {
        pair = &list.value->items[i];
        if (is_pair(pair->items[0].text, pair->items[1].text, two)) {
            *said = (said_t){ pair, list.file, list.line };
            return (true);
        }
    }
    return (false);
}

static bool add_order(host_t *h, const value_t *pair)
{
    const char *two[] = { pair->items[0].text, pair->items[1].text };
    found_t f = {};
    bridge_order_t *more = nullptr;
    char key[256];

    for (uint32_t i = 0; i < h->order_count; i++)
        if (is_pair(h->order[i].before, h->order[i].after, two))
            return (true);
    f = config_find(&h->config, states_pair, two);
    snprintf(key, sizeof(key), "order %s %s", two[0], two[1]);
    more = realloc(h->order, sizeof(*more) * (h->order_count + 1));
    if (!more)
        return (false);
    h->order = more;
    if (!config_say_found(&h->config, key, &f, ""))
        return (false);
    h->order[h->order_count++] = (bridge_order_t){
        f.at.value->items[0].text, f.at.value->items[1].text };
    return (true);
}

bool config_order(host_t *h)
{
    said_t said = {};
    bool taken = true;

    for (uint32_t f = 0; f < h->config.count; f++) {
        if (!config_states_key(&h->config.files[f], &ORDERED, &said))
            continue;
        for (uint32_t i = 0; i < said.value->count; i++)
            taken = add_order(h, &said.value->items[i]) && taken;
    }
    return (taken);
}
