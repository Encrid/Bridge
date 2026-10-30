/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "schema_test.h"

constexpr uint32_t SCHEMA_TEST_ITEMS = 20;

static bool read_many(enum bridge_kind kind, const char *start,
    const char *item)
{
    char text[512];
    char why[200];
    schema_source_t source = { kind, text, nullptr, nullptr, why, sizeof(why) };
    schema_t s;
    uint32_t i = 0;

    snprintf(text, sizeof(text), "%s", start);
    while (i < SCHEMA_TEST_ITEMS) {
        strcat(text, item);
        i++;
    }
    strcat(text, "}");
    if (schema_parse(&source, &s)) {
        printf("refused: %s\n", why);
        return (false);
    }
    schema_free(&s);
    return (true);
}

int schema_run_room(void)
{
    bool fields = read_many(BRIDGE_STRUCT, "{ ", "int32_t a; ");
    bool constants = fields && read_many(BRIDGE_ENUM, "uint8_t { ", "A, ");

    if (constants)
        printf("a struct of %u fields and an enum of %u constants were read\n",
            SCHEMA_TEST_ITEMS, SCHEMA_TEST_ITEMS);
    return (!constants);
}
