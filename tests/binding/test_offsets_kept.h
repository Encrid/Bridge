/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_OFFSETS_KEPT_H
    #define TEST_OFFSETS_KEPT_H

    #include "test_offsets_acc.h"

static inline int bridge_has(uint32_t slot, uint64_t id)
{
    return (acc_has(slot, (uint32_t)id));
}

#endif
