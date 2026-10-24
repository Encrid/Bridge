/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "os.h"

static uint64_t get_step(void)
{
    uint64_t before = os_get_now();
    uint64_t after = os_get_now();

    while (after == before)
        after = os_get_now();
    return (after - before);
}

int main(int argc, char **argv)
{
    uint64_t least = UINT64_MAX;
    uint64_t bound;
    uint64_t step;

    if (argc != 2)
        return (printf("usage: clock <bound in nanoseconds>\n"), 2);
    bound = strtoull(argv[1], nullptr, 10);
    for (int i = 0; i < 1000; i++) {
        step = get_step();
        if (step < least)
            least = step;
    }
    printf("step %s %llu ns\n", least < bound ? "below" : "not below",
        (unsigned long long)bound);
    return (0);
}
