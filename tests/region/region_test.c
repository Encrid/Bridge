/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "region.h"

typedef struct {
    const char *name;
    int numbers;
} step_t;

static const step_t STEPS[] = { { "room", 1 }, { "give", 2 }, { "bump", 1 },
    { "take", 1 }, { "grow", 3 }, { "most", 0 } };

constexpr size_t STEP_COUNT = sizeof(STEPS) / sizeof(*STEPS);

static void print_state(const region_t *r)
{
    uint32_t i = 0;

    printf("used %u peak %u gaps %u:", (unsigned)r->used, (unsigned)r->peak,
        (unsigned)r->count);
    while (i < r->count) {
        printf(" %u+%u", (unsigned)r->gaps[i].at, (unsigned)r->gaps[i].len);
        i++;
    }
    printf("\n");
}

static int read_step(char **words, int left, uint32_t *v)
{
    size_t i = 0;
    int k = 0;

    while (i < STEP_COUNT && strcmp(STEPS[i].name, words[0]))
        i++;
    if (i == STEP_COUNT || STEPS[i].numbers >= left)
        return (-1);
    while (k < STEPS[i].numbers) {
        v[k] = (uint32_t)strtoul(words[k + 1], nullptr, 10);
        k++;
    }
    return (k);
}

static void run_change(region_t *r, const char *step, const uint32_t *v)
{
    if (!strcmp(step, "room"))
        printf("room %u: %s\n", (unsigned)v[0],
            region_make_room(r, v[0]) ? "made" : "none");
    if (!strcmp(step, "give"))
        region_give(r, v[0], v[1]);
    if (!strcmp(step, "bump"))
        region_bump(r, v[0]);
}

static void run_ask(region_t *r, const char *step, const uint32_t *v)
{
    uint32_t at = 0;

    if (!strcmp(step, "take") && region_take(r, v[0], &at))
        printf("take %u: at %u\n", (unsigned)v[0], (unsigned)at);
    else if (!strcmp(step, "take"))
        printf("take %u: none\n", (unsigned)v[0]);
    if (!strcmp(step, "grow"))
        printf("grow %u+%u to %u: %s\n", (unsigned)v[0], (unsigned)v[1],
            (unsigned)v[2], region_grow(r, v[0], v[1], v[2]) ? "grown" : "none");
    if (!strcmp(step, "most"))
        printf("most %u\n", (unsigned)region_get_most(r));
}

int main(int argc, char **argv)
{
    region_t r = {};
    uint32_t v[3] = {};
    int i = 2;
    int numbers;

    r.len = argc > 1 ? (uint32_t)strtoul(argv[1], nullptr, 10) : 0;
    while (i < argc) {
        numbers = read_step(argv + i, argc - i, v);
        if (numbers < 0)
            return (printf("no step '%s'\n", argv[i]), 2);
        run_change(&r, argv[i], v);
        run_ask(&r, argv[i], v);
        i += numbers + 1;
    }
    print_state(&r);
    free(r.gaps);
    return (0);
}
