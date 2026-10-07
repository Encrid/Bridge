/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/storage/storage_test_hear.c -- the sinks of the program: what is
** printed of a record, and what is kept of it in the fixture.
*/

#include <stdio.h>
#include <string.h>

#include "storage_test.h"

/*
** storage_test_hear
**
** A sink: prints a record with whose it is and its two numbers.
*/
void storage_test_hear([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    printf("  %s: %.*s %u %u\n", r->system ? r->system : "-", (int)r->len,
        r->text, r->arg[0], r->arg[1]);
}

/*
** storage_test_hear_nothing
**
** A sink: drops the record. What the stress run says is the stress case's
** to judge, and not a measurement's.
*/
void storage_test_hear_nothing([[maybe_unused]] void *ud,
    [[maybe_unused]] const bridge_record_t *r)
{
}

/*
** storage_test_hear_memory
**
** A sink: counts, in the fixture, the records that say memory ran out or
** that the operating system would not give some, and prints every record,
** so that what each refusal said can be read.
*/
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

/*
** storage_test_hear_counts
**
** A sink: keeps, in the fixture, the two numbers a system said, and prints
** a warning or an error that is nobody's.
*/
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
