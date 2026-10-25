/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "grab.h"

static note_t *refuse(int ok)
{
    return (ok ? note_new() : nullptr);
}

static int tickets(bool dropped)
{
    ticket_t ticket = ticket_make(1);

    (void)ticket_make(0);
    ticket_drop((ticket_t){});
    if (dropped)
        ticket_drop(ticket);
    return (0);
}

static int parts(void)
{
    part_t *part = nullptr;

    note_delete(part_new(0, &part));
    note_delete(part_new(1, nullptr));
    note_delete(part_new(1, &part));
    part_delete(part);
    return (0);
}

static int texts(void)
{
    text_t one = {};
    text_t two;
    text_t three = {};

    text_fill(&one);
    text_fill(&two);
    text_join(&one, &two);
    text_fill(&three);
    text_delete(&three);
    text_delete(&three);
    text_delete(nullptr);
    return (0);
}

static int hands(void)
{
    int handed = note_hand(refuse, 1) + note_hand(refuse, 0) + note_hand(pool_refuse, 1);

    return (handed != 2);
}

int kinds(const char *mode)
{
    if (!strcmp(mode, "tickets"))
        return (tickets(true));
    if (!strcmp(mode, "stub"))
        return (tickets(false));
    if (!strcmp(mode, "parts"))
        return (parts());
    if (!strcmp(mode, "texts"))
        return (texts());
    if (!strcmp(mode, "hands"))
        return (hands());
    return (-1);
}
