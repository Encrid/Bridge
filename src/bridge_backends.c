/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/bridge_backends.c -- the backends of a deployment, opened one after
** another, and what all of them together need of a block they map.
*/

#include "bridge_inside.h"

/*
** get_common_multiple
**
** Answers the least length that is a whole number of both.
*/
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

/*
** take_shape
**
** Asks a backend what it can do and what it needs of a block it maps, and
** widens what the deck reserves to it. Every backend maps the shared
** block, so its length is a multiple of each one's granule.
*/
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

/*
** bridge_open_backends
**
** Opens every backend the deployment configures, in the order it states
** them. It configures one at least: a module is run by a backend, so a
** deck with none could load nothing.
**
** create.md, *Create*
*/
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
