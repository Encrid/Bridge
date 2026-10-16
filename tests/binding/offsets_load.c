/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "test_offsets_acc.h"

int32_t acc_ld_i8(uint32_t at)
{
    int8_t v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}

uint32_t acc_ld_u8(uint32_t at)
{
    uint8_t v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}

int32_t acc_ld_i16(uint32_t at)
{
    int16_t v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}

uint32_t acc_ld_u16(uint32_t at)
{
    uint16_t v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}

uint32_t acc_ld_32(uint32_t at)
{
    uint32_t v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}
