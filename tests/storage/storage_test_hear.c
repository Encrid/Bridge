/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

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

void storage_test_hear_memory(void *ud, const bridge_record_t *r)
{
    storage_test_t *t = ud;
    uint32_t i = 0;

    printf("%.*s\n", (int)r->len, r->text);
    while (i + 13 <= r->len) {
        t->said_memory += !memcmp(r->text + i, "out of memory", 13);
        t->said_memory += !memcmp(r->text + i, "would not give", 13);
        i++;
    }
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
