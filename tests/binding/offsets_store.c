/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "test_offsets_acc.h"

void acc_set_8(uint32_t slot, uint32_t a, uint32_t b, uint32_t field,
    uint32_t v)
{
    uint8_t narrow = (uint8_t)v;

    memcpy(test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(narrow), 1u), &narrow,
        sizeof(narrow));
}

void acc_set_16(uint32_t slot, uint32_t a, uint32_t b, uint32_t field,
    uint32_t v)
{
    uint16_t narrow = (uint16_t)v;

    memcpy(test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(narrow), 1u), &narrow,
        sizeof(narrow));
}

void acc_set_32(uint32_t slot, uint32_t a, uint32_t b, uint32_t field,
    uint32_t v)
{
    memcpy(test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(v), 1u), &v, sizeof(v));
}

void acc_set_64(uint32_t slot, uint32_t a, uint32_t b, uint32_t field,
    uint64_t v)
{
    memcpy(test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(v), 1u), &v, sizeof(v));
}

void acc_set_f32(uint32_t slot, uint32_t a, uint32_t b, uint32_t field, float v)
{
    memcpy(test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(v), 1u), &v, sizeof(v));
}
