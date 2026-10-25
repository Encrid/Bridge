/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "box.h"
#include "grab.h"
#include "pool.h"
#if defined(_WIN32)
    #include <io.h>
#else
    #include <unistd.h>
#endif

static void dropped(void (*drop)(void *), void *block)
{
    drop(block);
}

static int whole(void)
{
    char *kept = malloc(8);
    char *moved = realloc(nullptr, 8);
    char *zeroed = calloc(2, 4);
    void *pooled = pool_take();
    void *wrapped = wrapped_take();
    box_t *box = box_open(3);

    free(kept);
    dropped(free, moved);
    free(zeroed);
    free(pooled);
    free(wrapped);
    return (box_close(box) + plain(-1));
}

static int spoiled(void)
{
    FILE *lines = fopen(getenv("C23_LEAKS"), "a");

    if (!lines)
        return (1);
    fputs("src/none.c 1 2 3\n", lines);
    fputs("src/main.c 5\n", lines);
    fclose(lines);
    return (0);
}

static int misfit(void)
{
    box_t *box = box_open(1);
    char *text = box_format("%d", box_number(box_count(), box));

    free(text);
    box_close(box);
    return (0);
}

static int served(void)
{
    char *kept = malloc(8);

    printf("ready\n");
    fflush(stdout);
    while (getchar() != EOF)
        continue;
    return (kept == nullptr);
}

static int chosen(const char *mode)
{
    if (!strcmp(mode, "kept"))
        return (malloc(4) == nullptr);
    if (!strcmp(mode, "zero")) {
        free(realloc(nullptr, 0));
        return (0);
    }
    if (!strcmp(mode, "foreign")) {
        free(seize());
        return (0);
    }
    if (!strcmp(mode, "named")) {
        free(grab());
        return (0);
    }
    if (!strcmp(mode, "boxed"))
        return (box_open(2) == nullptr);
    if (!strcmp(mode, "spoiled"))
        return (spoiled());
    if (!strcmp(mode, "misfit"))
        return (misfit());
    if (!strcmp(mode, "served"))
        return (served());
    return (whole());
}

int main(int count, char **words)
{
    const char *mode = count > 1 ? words[1] : "whole";

    printf("%s\n", mode);
    fflush(stdout);
    return (chosen(mode));
}
