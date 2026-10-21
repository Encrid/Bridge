/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "config.h"
#include "storage_test.h"

static void try_bad_kind(enum schema_kind kind)
{
    char why[256] = "";
    schema_field_t field = { "setting", 0, 1, kind, nullptr };
    declared_type_t type = {};
    bridge_setting_t s = { "setting", BRIDGE_BOOL, {}, {} };
    config_seed_t seed = { .c = &type, .why = why, .why_len = sizeof(why) };

    type.s.fields = &field;
    type.s.field_count = 1;
    printf("kind %u: %s\n", (unsigned)kind,
        config_seed_setting(&seed, &s) ? why : "seeded");
}

enum storage_test_end storage_test_config_bad_kind(storage_test_t *t)
{
    (void)t;
    try_bad_kind(0);
    try_bad_kind(SCHEMA_STRUCT + 1);
    return (STORAGE_TEST_LEFT);
}
