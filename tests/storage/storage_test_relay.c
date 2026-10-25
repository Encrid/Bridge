/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "abi_declare.h"
#include "storage_test.h"

enum storage_test_end storage_test_relay_bad_kind(storage_test_t *t)
{
    declared_type_t type = { .size = 42 };
    schema_field_t field = { "setting", 0, 1,
        (enum schema_kind)(SCHEMA_STRUCT + 1), &type };

    (void)t;
    printf("element size of a bad kind: %u\n", abi_get_element_size(&field));
    return (STORAGE_TEST_DONE);
}
