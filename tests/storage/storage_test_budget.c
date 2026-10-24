/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

static enum storage_test_end create_with(storage_test_t *t)
{
    (void)bridge_destroy(t->w);
    t->w = bridge_create(&t->cfg, storage_test_hear, t);
    if (!t->w) {
        printf("the budget was refused\n");
        return (STORAGE_TEST_LEFT);
    }
    printf("the budget was taken\n");
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_count_types(storage_test_t *t)
{
    t->cfg.budget.types = UINT32_MAX;
    return (create_with(t));
}

enum storage_test_end storage_test_count_all(storage_test_t *t)
{
    t->cfg.budget.tables = UINT32_MAX - 8;
    return (create_with(t));
}
