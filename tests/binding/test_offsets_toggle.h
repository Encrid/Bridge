/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_OFFSETS_TOGGLE_H
    #define TEST_OFFSETS_TOGGLE_H

    #include "test_offsets_acc.h"

static inline int bridge_is_on(uint32_t slot, uint32_t table, uint32_t row)
{
    return (acc_is_on(slot, table, row));
}

static inline void bridge_set_toggle(uint32_t slot, uint32_t table,
    uint32_t row, int on)
{
    acc_set_toggle(slot, table, row, on);
}

#endif
