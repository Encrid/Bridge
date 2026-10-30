/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

void storage_test_hear([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    printf("  %s: %.*s %u %u\n", r->system ? r->system : "-", (int)r->len,
        r->text, r->arg[0], r->arg[1]);
}

void storage_test_hear_nothing([[maybe_unused]] void *ud,
    [[maybe_unused]] const bridge_record_t *r)
{
}

void storage_test_hear_memory([[maybe_unused]] void *ud,
    const bridge_record_t *r)
{
    printf("%.*s\n", (int)r->len, r->text);
}

void storage_test_hear_counts(void *ud, const bridge_record_t *r)
{
    storage_test_t *t = ud;

    if (r->system) {
        t->said[0] = r->arg[0];
        t->said[1] = r->arg[1];
        return;
    }
    if (r->level <= BRIDGE_WARN)
        printf("  -: %.*s\n", (int)r->len, r->text);
}
