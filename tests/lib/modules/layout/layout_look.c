/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "layout.h"

constexpr uint32_t ROOM = 8;

static void say(const test_api_t *api, const char *text, uint32_t first,
    uint32_t second)
{
    bridge_line_t line = { text, (uint32_t)strlen(text), first, second };

    test_log(api, BRIDGE_LOG_INFO, &line);
}

static void add_plain(sums_t *sums, const x_t *value)
{
    uint32_t k = 0;

#if V == 15
    sums->a += value->a2;
    sums->b += value->b2;
    while (k < sizeof(value->arr2)) {
        sums->arr += value->arr2[k];
        k++;
    }
    sums->mode += (uint32_t)value->mode2;
#else
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
#endif
}

static void add_rest(sums_t *sums, const x_t *value)
{
#if V == 15
    sums->at_x += value->at2.x;
    sums->at_y += value->at2.y;
#elif V == 17
    sums->at_x += value->at[0].x;
    sums->at_y += value->at[0].y;
    sums->more += value->at[1].x;
#else
    sums->at_x += value->at.x;
#if V != 11
    sums->at_y += value->at.y;
#endif
#if V == 8
    sums->more += value->at.z;
#elif V == 1 || V == 20 || V == 21
    sums->more += value->added;
#endif
#endif
}

TEST(look)
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
