/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "toml.h"

static bool copy_digits(const char *at, size_t len, char *out)
{
    size_t put = 0;

    for (size_t i = 0; i < len; i++) {
        if (at[i] == '_' && (!i || !isdigit((unsigned char)at[i - 1])
            || !isdigit((unsigned char)at[i + 1])))
            return (false);
        if (at[i] != '_')
            out[put++] = at[i];
    }
    out[put] = 0;
    return (true);
}

static bool has_zero_first(const char *digits)
{
    const char *at = digits + (*digits == '+' || *digits == '-');

    return (at[0] == '0' && isdigit((unsigned char)at[1]));
}

static bool read_whole(const char *digits, value_t *v)
{
    char *end = nullptr;

    errno = 0;
    *v = (value_t){ .kind = VALUE_WHOLE, .whole = strtoll(digits, &end, 10) };
    return (end != digits && !*end && !errno);
}

static bool read_real(const char *digits, value_t *v)
{
    char *end = nullptr;

    *v = (value_t){ .kind = VALUE_REAL, .real = strtod(digits, &end) };
    return (end != digits && !*end && isdigit((unsigned char)end[-1]));
}

bool toml_read_number(reading_t *r, value_t *v)
{
    char digits[64];
    size_t len = strspn(r->at, "+-0123456789_.eE");
    bool taken = len < sizeof(digits) && copy_digits(r->at, len, digits);

    if (taken && (r->at[len] == ':' || islower((unsigned char)r->at[len])))
        taken = false;
    if (taken && has_zero_first(digits))
        taken = false;
    if (taken && strpbrk(digits, ".eE"))
        taken = read_real(digits, v);
    else if (taken)
        taken = read_whole(digits, v);
    if (!taken)
        return (toml_refuse(r, "a number that is not taken"));
    r->at += len;
    return (true);
}
