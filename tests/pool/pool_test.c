/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "pool_test.h"
#include "sweep.h"

constexpr int MOST_WORDS = 8;

static uint32_t read_number(const char *word)
{
    return ((uint32_t)strtoul(word, nullptr, 10));
}

static void sweep_create(const char *call, uint32_t workers)
{
    unsigned at = 0;
    pool_t *p;

    do {
        at++;
        sweep_refuse(call, at);
        p = pool_create(workers, 2, POOL_TEST_STACK);
        if (sweep_is_refused())
            printf("%s %u refused: %s\n", call, at, p ? "a pool" : "no pool");
        pool_destroy(p);
    } while (sweep_is_refused());
    sweep_refuse(call, 0);
    printf("%u refused calls of %s, then a pool\n", at - 1, call);
}

static void run_row(char **words, int count)
{
    const char *mode = count ? words[0] : "";

    if (!strcmp(mode, "run") && count > 2)
        pool_test_rounds(read_number(words[1]), read_number(words[2]),
            count > 3 ? words[3] : "");
    else if (!strcmp(mode, "meet") && count > 1)
        (void)pool_test_meet(read_number(words[1]));
    else if (!strcmp(mode, "sweep") && count > 2)
        sweep_create(words[1], read_number(words[2]));
    else if (!strcmp(mode, "none")) {
        pool_destroy(nullptr);
        printf("no pool to end\n");
    } else
        printf("no mode '%s'\n", mode);
}

int main(void)
{
    char line[256];
    char *words[MOST_WORDS];
    char *word;
    int count;

    pool_test_start();
    while (fgets(line, sizeof(line), stdin)) {
        count = 0;
        word = strtok(line, " \r\n");
        while (word && count < MOST_WORDS) {
            words[count++] = word;
            word = strtok(nullptr, " \r\n");
        }
        run_row(words, count);
        printf("\n");
    }
    return (0);
}
