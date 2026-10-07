/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/reload_check.c -- what a reload is refused for before anything is
** made.
*/

#include <stdarg.h>
#include <stdio.h>

#include "reload.h"

/*
** reload_refuse
**
** Says that the loaded module is not reloaded, and why. Answers 1.
**
** failure.md, *The budget*
*/
int reload_refuse(const reloading_t *r, const char *format, ...)
{
    char text[512];
    va_list args;

    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    say_error(r->b, "module '%s' is not reloaded: %s", r->old->identity, text);
    return (1);
}

/*
** reload_check
**
** Refuses a reload of a module that is not loaded, that may not be
** reloaded, or that names another backend than it runs on. hot_reload only
** ever forbids.
**
** wasm.md, *Exports*
*/
int reload_check(reloading_t *r)
{
    module_check_t k = { r->b, r->a };
    const char *id = r->a->cfg.identity;

    if (bridge_check_call(r->b, id) || module_check(&k))
        return (1);
    r->on = k.on;
    r->demands = k.demands;
    r->old = module_find(r->b, id);
    if (!r->old) {
        say_error(r->b, "module '%s' is not reloaded: nothing of that name "
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
