/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_inside.h"

static uint64_t get_common_multiple(uint64_t a, uint64_t b)
{
    uint64_t x = a;
    uint64_t y = b;
    uint64_t rest;

    while (y) {
        rest = x % y;
        x = y;
        y = rest;
    }
    return (a / x * b);
}

static void take_shape(bridge_t *b, open_backend_t *o)
{
    uint64_t reservation = 0;
    uint64_t guard = 0;
    uint64_t granule = 0;

    o->capabilities = o->api.backend_get_capabilities(o->be);
    o->api.memory_get_shape(o->be, &reservation, &guard, &granule);
    o->reservation = reservation;
    o->guard = guard;
    o->granule = granule ? granule : 1;
    if (reservation > b->reservation)
        b->reservation = reservation;
    if (guard > b->guard)
        b->guard = guard;
    b->granule = get_common_multiple(b->granule, o->granule);
}

int bridge_open_backends(bridge_t *b, const bridge_config_t *cfg)
{
    uint32_t i = 0;

    if (!cfg->backend_count) {
        say_error(b, "the deployment configures no backend, and a module "
            "is run by one");
        return (1);
    }
    while (i < cfg->backend_count) {
        if (bridge_open_backend(b, cfg, i))
            return (1);
        take_shape(b, &b->backends[i]);
        i++;
    }
    return (0);
}
