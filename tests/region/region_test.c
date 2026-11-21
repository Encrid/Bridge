/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "region.h"
#include "sweep.h"

typedef struct {
    const char *name;
    int numbers;
} step_t;

static const step_t STEPS[] = { { "room", 1 }, { "give", 2 }, { "bump", 1 },
    { "take", 1 }, { "grow", 3 }, { "most", 0 } };

constexpr size_t STEP_COUNT = sizeof(STEPS) / sizeof(*STEPS);
constexpr int MOST_WORDS = 40;

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

static void run_step(region_t *r, const char *step, const uint32_t *v)
{
    uint32_t at = 0;

    if (!strcmp(step, "room"))
        printf("room %u: %s\n", (unsigned)v[0],
            region_make_room(r, v[0]) ? "made" : "none");
    if (!strcmp(step, "give"))
        region_give(r, v[0], v[1]);
    if (!strcmp(step, "bump"))
        region_bump(r, v[0]);
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

static void run_row(char **words, int count)
{
    region_t r = {};
    uint32_t v[3] = {};
    int numbers = 0;
    int i = 1;

    r.len = count ? (uint32_t)strtoul(words[0], nullptr, 10) : 0;
    while (i < count && numbers >= 0) {
        numbers = read_step(words + i, count - i, v);
        if (numbers < 0)
            printf("no step '%s'\n", words[i]);
        else
            run_step(&r, words[i], v);
        i += numbers + 1;
    }
    print_state(&r);
    free(r.gaps);
}

int main(void)
{
    char line[512];
    char *words[MOST_WORDS];
    char *word;
    int count;

    while (fgets(line, sizeof(line), stdin)) {
        count = 0;
        word = strtok(line, " \r\n");
        while (word && count < MOST_WORDS) {
            words[count++] = word;
            word = strtok(nullptr, " \r\n");
        }
        if (count > 3 && !strcmp(words[0], "refuse")) {
            sweep_refuse(words[1], (unsigned)strtoul(words[2], nullptr, 10));
            run_row(words + 3, count - 3);
            sweep_refuse(words[1], 0);
        } else
            run_row(words, count);
        printf("\n");
    }
    return (0);
}
