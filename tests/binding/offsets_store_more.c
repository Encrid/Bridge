/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "test_offsets_acc.h"

void acc_set_f64(uint32_t slot, uint32_t a, uint32_t b, uint32_t field,
    double v)
{
    memcpy(test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(v), 1u), &v, sizeof(v));
}

void acc_set_own_32(uint32_t at, uint32_t v)
{
    memcpy(test_get_table()->private_base + at, &v, sizeof(v));
}

void acc_copy_to_private(uint32_t to, const void *from, uint32_t len)
{
    memcpy(test_get_table()->private_base + to, from, len);
}

uint32_t acc_is_on(uint32_t slot, uint32_t table, uint32_t row)
{
    uint32_t at = acc_get_toggle_bits(slot, table, row);

    return ((uint32_t)(acc_get_shared_64(at) >> (row & 63u)) & 1u);
}

void acc_set_toggle(uint32_t slot, uint32_t table, uint32_t row, uint32_t on)
{
    uint32_t at;
    uint64_t bit;
    uint64_t word;

    if (!acc_get_own_32(BRIDGE_HEADER(write) + slot * 4u))
        acc_trap();
    at = acc_get_toggle_bits(slot, table, row);
    bit = (uint64_t)1 << (row & 63u);
    word = acc_get_shared_64(at);
    word = on ? word | bit : word & ~bit;
    memcpy(test_get_table()->shared_base + at, &word, sizeof(word));
}
