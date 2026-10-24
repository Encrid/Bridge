/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdarg.h>
#include <stdio.h>

#include "module.h"

int module_refuse(bridge_world_t *w, const char *identity,
    const char *format, ...)
{
    char text[512];
    va_list args;

    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    say_error(w, "module '%s' is refused: %s", identity, text);
    return (1);
}

int module_fail(backend_err_t *err, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(err->text, sizeof(err->text), format, args);
    va_end(args);
    return (1);
}

int module_check_set(const module_check_t *k, const char *key, int64_t value)
{
    if (value)
        return (0);
    return (module_refuse(k->w, k->id, "%s is unset", key));
}

int module_check_bound(const module_check_t *k, const char *key,
    int64_t value, int64_t none)
{
    const char *either = "an amount nor BRIDGE_NONE";

    if (value > 0 || value == none)
        return (0);
    if (none == BRIDGE_NO_LIMIT)
        either = "a limit nor BRIDGE_NO_LIMIT";
    return (module_refuse(k->w, k->id, "%s is %lld, which is neither %s", key,
        (long long)value, either));
}

int module_check_flag(const module_check_t *k, const char *key,
    enum bridge_flag value)
{
    if (value == BRIDGE_TRUE || value == BRIDGE_FALSE)
        return (0);
    if (value == BRIDGE_UNSET)
        return (module_refuse(k->w, k->id, "%s is unset", key));
    return (module_refuse(k->w, k->id, "%s is %u, which is neither "
        "BRIDGE_TRUE nor BRIDGE_FALSE", key, value));
}
