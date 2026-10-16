/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "storage_test.h"

static enum storage_test_end make_matrix(uint32_t n)
{
    schedule_matrix_t m = {};
    bool made = schedule_init_matrix(&m, n);

    printf("a matrix for %u systems: %s, %u words a row\n", m.n,
        made ? "made" : "not made", m.words);
    free(m.bits);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_make_large_matrix(
    [[maybe_unused]] storage_test_t *t)
{
    return (make_matrix(4'294'967'233u));
}

enum storage_test_end storage_test_make_small_matrix(
    [[maybe_unused]] storage_test_t *t)
{
    return (make_matrix(64));
}
