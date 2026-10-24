/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdarg.h>
#include <stdio.h>

#include "reload.h"

int reload_refuse(const reloading_t *r, const char *format, ...)
{
    char text[512];
    va_list args;

    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    say_error(r->w, "module '%s' is not reloaded: %s", r->old->identity, text);
    return (1);
}

int reload_check(reloading_t *r)
{
    module_check_t k = { r->w, r->a };
    const char *id = r->a->cfg.identity;

    if (bridge_check_call(r->w, id) || module_check(&k))
        return (1);
    r->on = k.on;
    r->demands = k.demands;
    r->old = module_find(r->w, id);
    if (!r->old) {
        say_error(r->w, "module '%s' is not reloaded: nothing of that name "
            "is loaded", id);
        return (1);
    }
    if (!r->old->hot_reload)
        return (reload_refuse(r, "its hot_reload is false"));
    if (r->on != r->old->on)
        return (reload_refuse(r, "it runs on backend '%s' and a reload does "
            "not move it to '%s'", r->old->on->name, r->on->name));
    return (0);
}
