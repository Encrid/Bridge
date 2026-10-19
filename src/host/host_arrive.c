/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static bool is_trusted(const file_t *f, const char *backend)
{
    char answer[64] = "";

    printf("? trust %s, %s, %s\n", f->of->who.identity, backend, f->path);
    fflush(stdout);
    if (!fgets(answer, sizeof(answer), stdin))
        return (false);
    answer[strcspn(answer, "\r\n")] = 0;
    return (!strcmp(answer, "yes"));
}

bool may_arrive(const file_t *f, const held_t *held)
{
    bool may = held->arrive == ARRIVE_LOAD;

    if (held->arrive == ARRIVE_ASK)
        may = is_trusted(f, held->name);
    if (!may)
        printf("not loaded: %s: %s, %s, not trusted\n", f->path,
            f->of->who.identity, held->name);
    return (may);
}
