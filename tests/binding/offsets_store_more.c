/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "test_offsets_acc.h"

void acc_st_f64(uint32_t at, double v)
{
    memcpy(test_get_table()->shared_base + at, &v, sizeof(v));
}

void acc_own_st(uint32_t at, uint32_t v)
{
    memcpy(test_get_table()->private_base + at, &v, sizeof(v));
}

void acc_to_private(uint32_t to, const void *from, uint32_t len)
{
    memcpy(test_get_table()->private_base + to, from, len);
}
