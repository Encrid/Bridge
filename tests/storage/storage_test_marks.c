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
    uint8_t queue[64] = {};
    module_t m = { .queue = queue, .queue_room = sizeof(queue) };
    change_t ch = {};

    if (!queue_change(&m, &ch, nullptr)) {
        printf("FAIL: a change with no room for its mark was queued\n");
        return (STORAGE_TEST_FAILED);
    }
    printf("refused: %s\n", m.why);
    return (STORAGE_TEST_DONE);
}
