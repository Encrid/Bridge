/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "grab.h"

struct note {
    int unused;
};

struct part {
    int unused;
};

char *grab(void)
{
    return (malloc(8));
}

char *seize(void)
{
    return (malloc(8));
}

note_t *note_new(void)
{
    return (malloc(sizeof(note_t)));
}

void note_delete(note_t *note)
{
    free(note);
}

note_t *part_new(int ok, part_t **out)
{
    if (!ok)
        return (note_new());
    if (out)
        *out = malloc(sizeof(part_t));
    return (nullptr);
}

void part_delete(part_t *part)
{
    free(part);
}

ticket_t ticket_make(int ok)
{
    static unsigned next;

    return ((ticket_t){ ok ? ++next : 0 });
}

void ticket_drop(ticket_t ticket)
{
    (void)ticket;
}

void text_fill(text_t *text)
{
    text->size = 4;
    text->data = malloc(4);
}

void text_delete(text_t *text)
{
    if (!text)
        return;
    free(text->data);
    *text = (text_t){};
}

void text_join(text_t *one, text_t *two)
{
    text_delete(one);
    text_delete(two);
}

int note_hand(note_t *(*make)(int ok), int ok)
{
    note_t *note = make(ok);

    note_delete(note);
    return (note != nullptr);
}
