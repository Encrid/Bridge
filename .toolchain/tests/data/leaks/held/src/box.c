/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "box.h"

struct box {
    int size;
};

static int opened;

box_t *box_open(int size)
{
    box_t *box = malloc(sizeof(*box));

    if (box) {
        box->size = size;
        opened++;
    }
    return (box);
}

int box_close(box_t *box)
{
    if (box)
        opened--;
    free(box);
    return (opened);
}

int box_count(void)
{
    return (opened);
}

int box_number(int base, box_t *box)
{
    return (base + (box ? box->size : 0));
}

char *box_format(const char *text, ...)
{
    va_list list;
    char *out = malloc(64);

    if (!out)
        return (nullptr);
    va_start(list, text);
    vsnprintf(out, 64, text, list);
    va_end(list);
    return (out);
}
