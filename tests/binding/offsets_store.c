/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "test_offsets_acc.h"

void acc_st_8(uint32_t at, uint32_t v)
{
    uint8_t narrow = (uint8_t)v;
    uint8_t *to = test_get_table()->shared_base + at;

    memcpy(to, &narrow, sizeof(narrow));
}

void acc_st_16(uint32_t at, uint32_t v)
{
    uint16_t narrow = (uint16_t)v;
    uint8_t *to = test_get_table()->shared_base + at;

    memcpy(to, &narrow, sizeof(narrow));
}

void acc_st_32(uint32_t at, uint32_t v)
{
    memcpy(test_get_table()->shared_base + at, &v, sizeof(v));
}

void acc_st_64(uint32_t at, uint64_t v)
{
    memcpy(test_get_table()->shared_base + at, &v, sizeof(v));
}

void acc_st_f32(uint32_t at, float v)
{
    memcpy(test_get_table()->shared_base + at, &v, sizeof(v));
}
