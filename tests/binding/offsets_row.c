/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/binding/offsets_row.c -- the table of the call in progress, where
** a row is, and the end of a call.
**
** abi.md, *The ABI*
*/

#include <string.h>

#include "test_offsets_acc.h"

/*
** table_at
**
** Answers where the table is kept, which is once for each thread: a deck
** may have a library's module twice, and two decks the same module, each
** called on a thread of its own.
*/
static const test_api_t **table_at(void)
{
    static thread_local const test_api_t *table;

    return (&table);
}

/*
** test_keep_table
**
** Keeps the table of the call that is about to be made on this thread:
** the tests' backend calls it before each.
*/
TEST_EXPORT void test_keep_table(const test_api_t *api)
{
    *table_at() = api;
}

/*
** test_get_table
**
** Answers the table of the call in progress.
*/
const test_api_t *test_get_table(void)
{
    return (*table_at());
}

/*
** acc_trap
**
** Ends the call in progress: the module cannot go on.
*/
void acc_trap(void)
{
    const test_api_t *api = test_get_table();

    api->trap(api);
}

/*
** acc_row
**
** Answers where row `row` of a column is. Built with BRIDGE_ROWS_CHECKED,
** it ends the call if that is not one of the span's rows.
*/
uint32_t acc_row(uint32_t base, uint32_t row, [[maybe_unused]] uint32_t count,
    uint32_t stride)
{
#ifdef BRIDGE_ROWS_CHECKED
    if (row >= count)
        acc_trap();
#endif
    return (base + row * stride);
}
