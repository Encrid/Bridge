/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "test_offsets_acc.h"

static const test_api_t **table_at(void)
{
    static thread_local const test_api_t *table;

    return (&table);
}

TEST_EXPORT void test_keep_table(const test_api_t *api)
{
    *table_at() = api;
}

const test_api_t *test_get_table(void)
{
    return (*table_at());
}

void acc_trap(void)
{
    const test_api_t *api = test_get_table();

    api->trap(api);
}

uint32_t acc_row(uint32_t base, uint32_t row, [[maybe_unused]] uint32_t count,
    uint32_t stride)
{
#ifdef BRIDGE_ROWS_CHECKED
    if (row >= count)
        acc_trap();
#endif
    return (base + row * stride);
}
