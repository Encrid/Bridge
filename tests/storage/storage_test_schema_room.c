/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage_test.h"

#ifdef COUNTED

constexpr uint32_t STORAGE_TEST_ITEMS = 20;

static size_t count_allocations(enum bridge_kind kind, const char *start,
    const char *item, size_t fail_at)
{
    storage_test_allocator_t *allocator = storage_test_get_allocator();
    char text[512];
    char why[200];
    schema_source_t source = { kind, text, nullptr, nullptr, why, sizeof(why) };
    schema_t s;
    size_t asked;
    uint32_t i = 0;

    snprintf(text, sizeof(text), "%s", start);
    while (i < STORAGE_TEST_ITEMS) {
        strcat(text, item);
        i++;
    }
    strcat(text, "}");
    allocator->asked = 0;
    allocator->fail_at = fail_at;
    if (schema_parse(&source, &s)) {
        printf("refused: %s\n", why);
        return (0);
    }
    asked = allocator->asked;
    schema_free(&s);
    return (asked);
}

enum storage_test_end storage_test_count_room(storage_test_t *t)
{
    size_t fail_at = t->more ? (size_t)atoi(t->more) : 0;
    size_t fields = count_allocations(BRIDGE_STRUCT, "{ ", "int32_t a; ",
        fail_at);
    size_t constants = count_allocations(BRIDGE_ENUM, "uint8_t { ", "A, ",
        fail_at);

    storage_test_get_allocator()->fail_at = 0;
    printf("a struct of %u fields took %zu allocations, and an enum of %u "
        "constants took %zu\n", STORAGE_TEST_ITEMS, fields, STORAGE_TEST_ITEMS,
        constants);
    return (STORAGE_TEST_DONE);
}

#endif
