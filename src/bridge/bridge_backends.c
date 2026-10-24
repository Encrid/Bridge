/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_inside.h"

static constexpr uint64_t GRANULE_MOST = (uint64_t)1 << 30;
static constexpr uint64_t SPAN_MOST = (uint64_t)1 << 62;

static int refuse_span(bridge_world_t *w, const open_backend_t *o,
    const char *what, uint64_t bytes)
{
    say_error(w, "backend '%s' states a %s of %llu bytes, and the most is "
        "%llu", o->name, what, (unsigned long long)bytes,
        (unsigned long long)SPAN_MOST);
    return (1);
}

static int check_shape(bridge_world_t *w, const open_backend_t *o,
    uint64_t granule)
{
    if (granule & (granule - 1) || granule > GRANULE_MOST) {
        say_error(w, "backend '%s' states a granule of %llu, and a granule "
            "is 0, or a power of two up to %llu", o->name,
            (unsigned long long)granule, (unsigned long long)GRANULE_MOST);
        return (1);
    }
    if (o->reservation > SPAN_MOST)
        return (refuse_span(w, o, "reservation", o->reservation));
    if (o->guard > SPAN_MOST)
        return (refuse_span(w, o, "guard", o->guard));
    return (0);
}

static int take_shape(bridge_world_t *w, open_backend_t *o)
{
    uint64_t reservation = 0;
    uint64_t guard = 0;
    uint64_t granule = 0;

    o->capabilities = o->api.backend_get_capabilities(o->be);
    o->api.memory_get_shape(o->be, &reservation, &guard, &granule);
    o->reservation = reservation;
    o->guard = guard;
    o->granule = granule ? granule : 1;
    if (check_shape(w, o, granule))
        return (1);
    if (reservation > w->reservation)
        w->reservation = reservation;
    if (guard > w->guard)
        w->guard = guard;
    if (o->granule > w->granule)
        w->granule = o->granule;
    return (0);
}

int bridge_open_backends(bridge_world_t *w, const bridge_config_t *cfg)
{
    uint32_t i = 0;

    if (!cfg->backend_count) {
        say_error(w, "the deployment configures no backend, and a module "
            "is run by one");
        return (1);
    }
    while (i < cfg->backend_count) {
        if (bridge_open_backend(w, cfg, i) || take_shape(w, &w->backends[i]))
            return (1);
        i++;
    }
    return (0);
}
