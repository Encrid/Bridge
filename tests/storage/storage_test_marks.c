/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "abi_entry.h"
#include "storage_test.h"

enum storage_test_end storage_test_fill_marks([[maybe_unused]] storage_test_t *t)
{
    module_t m = {};

    if (!mark_turn(&m)) {
        printf("FAIL: a turn with no room for its mark was taken\n");
        return (STORAGE_TEST_FAILED);
    }
    printf("refused: %s\n", m.why);
    return (STORAGE_TEST_DONE);
}
