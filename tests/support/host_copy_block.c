/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "host_copy.h"

static void check_room(const host_copy_t *c, size_t size)
{
    if (c->block && c->low + size <= c->high)
        return;
    printf("the copy of a config needs more than %zu bytes\n",
        HOST_COPY_ROOM);
    exit(1);
}

void *host_take(host_copy_t *c, const void *from, size_t size)
{
    size_t room = (size + 7) & ~(size_t)7;
    void *to = nullptr;

    check_room(c, room);
    to = c->block + c->low;
    memcpy(to, from, size);
    c->low += room;
    return (to);
}

const char *host_take_text(host_copy_t *c, const char *text)
{
    size_t size = text ? strlen(text) + 1 : 0;

    if (!text)
        return (nullptr);
    check_room(c, size);
    c->high -= size;
    memcpy(c->block + c->high, text, size);
    return ((const char *)c->block + c->high);
}

void host_spoil(void **spoiled, host_copy_t *c)
{
    for (size_t i = c->high; i < HOST_COPY_ROOM; i++)
        if (c->block[i])
            c->block[i] = 'x';
    memcpy(c->block, spoiled, sizeof(*spoiled));
    *spoiled = c->block;
}

void host_free_spoiled(void **spoiled)
{
    void *next = nullptr;

    while (*spoiled) {
        memcpy(&next, *spoiled, sizeof(next));
        free(*spoiled);
        *spoiled = next;
    }
}
