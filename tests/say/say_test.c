/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "say.h"

static void write_record(void *ud, const bridge_record_t *r)
{
    (void)ud;
    printf("%u %s %s %u: %.*s\n", (unsigned)r->level,
        r->identity ? r->identity : "none", r->system ? r->system : "none",
        (unsigned)r->barriers, (int)r->len, r->text);
}

static void run_copy(void)
{
    char *copy = copy_text("a text that is cut here", 6);

    printf("%s\n", copy ? copy : "no copy");
    free(copy);
}

static void run_lacks(bridge_world_t *w)
{
    int got = bridge_lacks(w, w, "a table");
    int none = bridge_lacks(w, nullptr, "a table");

    printf("%d %d\n", got, none);
}

int main(int argc, char **argv)
{
    static bridge_world_t w;
    static module_t m;
    static bridge_system_t s;
    static char identity[] = "game:mod@1.0.0";
    static char name[] = "tick";
    const say_t about = { &m, &s, BRIDGE_WARN };
    const char *mode = argc > 1 ? argv[1] : "";

    w.sink = write_record;
    w.barriers = 4;
    m.identity = identity;
    s.name = name;
    if (!strcmp(mode, "error"))
        say_error(&w, "a %s of %d", "text", 7);
    if (!strcmp(mode, "about"))
        say_about(&w, &about, "a %s of %d", "text", 7);
    if (!strcmp(mode, "copy"))
        run_copy();
    if (!strcmp(mode, "lacks"))
        run_lacks(&w);
    return (0);
}
