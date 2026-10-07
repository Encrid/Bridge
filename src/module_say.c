/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/module_say.c -- how a refused module and a failed load are said, and
** the keys of a config that are checked one by one.
**
** config.md, *No unaccountable values*
*/

#include <stdarg.h>
#include <stdio.h>

#include "module.h"

/*
** module_refuse
**
** Says that a module is refused, and why. Answers 1.
**
** create.md, *The sink*
*/
int module_refuse(bridge_t *b, const char *identity, const char *format, ...)
{
    char text[512];
    va_list args;

    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    say_error(b, "module '%s' is refused: %s", identity, text);
    return (1);
}

/*
** module_fail
**
** Writes why a module's load or reload failed. Answers 1.
*/
int module_fail(backend_err_t *err, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(err->text, sizeof(err->text), format, args);
    va_end(args);
    return (1);
}

/*
** module_check_set
**
** Refuses a key whose zero means nobody stated it.
**
** config.md, *No unaccountable values*
*/
int module_check_set(const module_check_t *k, const char *key, int64_t value)
{
    if (value)
        return (0);
    return (module_refuse(k->b, k->id, "%s is unset", key));
}

/*
** module_check_bound
**
** Refuses a negative number that is not `none`.
**
** config.md, *No unaccountable values*
*/
int module_check_bound(const module_check_t *k, const char *key,
    int64_t value, int64_t none)
{
    const char *either = "an amount nor BRIDGE_NONE";

    if (value > 0 || value == none)
        return (0);
    if (none == BRIDGE_NO_LIMIT)
        either = "a limit nor BRIDGE_NO_LIMIT";
    return (module_refuse(k->b, k->id, "%s is %lld, which is neither %s", key,
        (long long)value, either));
}

/*
** module_check_flag
**
** Refuses a key that is yes or no and is stated as neither.
**
** config.md, *No unaccountable values*
*/
int module_check_flag(const module_check_t *k, const char *key,
    enum bridge_flag value)
{
    if (value == BRIDGE_TRUE || value == BRIDGE_FALSE)
        return (0);
    if (value == BRIDGE_UNSET)
        return (module_refuse(k->b, k->id, "%s is unset", key));
    return (module_refuse(k->b, k->id, "%s is %u, which is neither "
        "BRIDGE_TRUE nor BRIDGE_FALSE", key, value));
}
