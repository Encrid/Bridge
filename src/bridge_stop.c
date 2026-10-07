/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/bridge_stop.c -- a deck stopped by the loss of a module, and let run
** again.
*/

#include <stdio.h>
#include <string.h>

#include "bridge_inside.h"
#include "say.h"

/*
** bridge_get_stem_length
**
** Answers how long an identity is without its version: up to its '@'.
*/
size_t bridge_get_stem_length(const char *identity)
{
    const char *at = strchr(identity, '@');

    if (!at)
        return (strlen(identity));
    return ((size_t)(at - identity));
}

/*
** bridge_find_pinned
**
** Answers a loaded module that is pinned to another thread than the calling
** one, or nullptr.
*/
const module_t *bridge_find_pinned(const bridge_t *b)
{
    const module_t *m = b->modules;
    uint64_t thread = os_get_thread_id();

    while (m && (!m->pinned || m->thread == thread))
        m = m->next;
    return (m);
}

/*
** bridge_stop
**
** Stops the deck for the loss of a module stated `fail`, and keeps why and
** the module's name without its version. A deck that has stopped stays
** stopped for its first reason.
**
** failure.md, *When something fails*
*/
void bridge_stop(bridge_t *b, const char *identity, const char *what)
{
    int stem = (int)bridge_get_stem_length(identity);

    if (b->stopped_by)
        return;
    snprintf(b->stopped_why, sizeof(b->stopped_why),
        "module '%s' %s and its on_error is fail", identity, what);
    snprintf(b->stopped_name, sizeof(b->stopped_name), "%.*s", stem,
        identity);
    b->stopped_by = b->stopped_why;
    b->stopped_stem = b->stopped_name;
    say_error(b, "the deck has stopped: %s", b->stopped_why);
}

/*
** bridge_resume
**
** Lets the deck run again, if `m` is the module it was waiting for.
**
** failure.md, *When something fails*
*/
void bridge_resume(bridge_t *b, const module_t *m)
{
    say_t about = { m, nullptr, 0 };

    if (!b->stopped_stem || strcmp(b->stopped_stem, m->stem))
        return;
    b->stopped_by = nullptr;
    b->stopped_stem = nullptr;
    say_about(b, &about, "the deck runs again: module '%s' is back",
        m->identity);
}

/*
** bridge_give_up
**
** Lets the deck run again without the module that stopped it.
**
** failure.md, *When something fails*
*/
void bridge_give_up(bridge_t *b, const module_t *m)
{
    say_t about = { m, nullptr, 0 };

    if (!b->stopped_stem || strcmp(b->stopped_stem, m->stem))
        return;
    b->stopped_by = nullptr;
    b->stopped_stem = nullptr;
    say_about(b, &about, "the deck runs again: module '%s' is unloaded, "
        "and it goes on without it", m->identity);
}
