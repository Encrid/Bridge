/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "test_offsets_acc.h"

int32_t acc_get_i8(uint32_t slot, uint32_t a, uint32_t b, uint32_t field)
{
    int8_t v;

    memcpy(&v, test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(v), 0u), sizeof(v));
    return (v);
}

uint32_t acc_get_u8(uint32_t slot, uint32_t a, uint32_t b, uint32_t field)
{
    uint8_t v;

    memcpy(&v, test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(v), 0u), sizeof(v));
    return (v);
}

int32_t acc_get_i16(uint32_t slot, uint32_t a, uint32_t b, uint32_t field)
{
    int16_t v;

    memcpy(&v, test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(v), 0u), sizeof(v));
    return (v);
}

uint32_t acc_get_u16(uint32_t slot, uint32_t a, uint32_t b, uint32_t field)
{
    uint16_t v;

    memcpy(&v, test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(v), 0u), sizeof(v));
    return (v);
}

uint32_t acc_get_32(uint32_t slot, uint32_t a, uint32_t b, uint32_t field)
{
    uint32_t v;

    memcpy(&v, test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(v), 0u), sizeof(v));
    return (v);
}
