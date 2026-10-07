/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nlayout/nlayout_look.c -- the system of nlayout that
** says what the values of X are: every field this build has, summed.
*/

#include "nlayout.h"

constexpr uint32_t ROOM = 8;

static void say(const bridge_api_t *api, const char *text, uint32_t first,
    uint32_t second)
{
    bridge_line_t line = { text, (uint32_t)strlen(text), first, second };

    bridge_native_log(api, BRIDGE_LOG_INFO, &line);
}

/*
** add_plain
**
** Adds to the sums the fields of a value that are X's own: a, b under the
** name this build gives it and where it has one, every element of arr, and
** mode.
*/
static void add_plain(sums_t *sums, const x_t *value)
{
    uint32_t k = 0;

    sums->a += value->a;
#if V == 6
    sums->b += value->bee;
#elif V != 4
    sums->b += value->b;
#endif
    while (k < sizeof(value->arr)) {
        sums->arr += value->arr[k];
        k++;
    }
    sums->mode += (uint32_t)value->mode;
}

/*
** add_rest
**
** Adds to the sums the fields of the struct `at` names, those this build
** has, and the field a build added: to that struct, or to X.
*/
static void add_rest(sums_t *sums, const x_t *value)
{
    sums->at_x += value->at.x;
#if V != 11
    sums->at_y += value->at.y;
#endif
#if V == 8
    sums->more += value->at.z;
#elif V == 1
    sums->more += value->added;
#endif
}

/*
** look
**
** Says how many values of X there are, and the sum of every field this
** build has over them.
*/
BRIDGE_NATIVE(look)
{
    const x_t *value[ROOM];
    sums_t sums = {};
    uint32_t n = collect_values(api, value, ROOM);
    uint32_t i = 0;

    while (i < n) {
        add_plain(&sums, value[i]);
        add_rest(&sums, value[i]);
        i++;
    }
    say(api, "values, and the sum of a", n, sums.a);
    say(api, "the sums of b, and of arr", sums.b, sums.arr);
    say(api, "the sums of at.x, and of at.y", sums.at_x, sums.at_y);
    say(api, "the sums of mode, and of what was added", sums.mode, sums.more);
    return (0);
}
