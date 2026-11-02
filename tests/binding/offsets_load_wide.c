/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "test_offsets_acc.h"

uint64_t acc_get_64(uint32_t slot, uint32_t a, uint32_t b, uint32_t field)
{
    uint64_t v;

    memcpy(&v, test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(v), 0u), sizeof(v));
    return (v);
}

float acc_get_f32(uint32_t slot, uint32_t a, uint32_t b, uint32_t field)
{
    float v;

    memcpy(&v, test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(v), 0u), sizeof(v));
    return (v);
}

double acc_get_f64(uint32_t slot, uint32_t a, uint32_t b, uint32_t field)
{
    double v;

    memcpy(&v, test_get_table()->shared_base
        + acc_get_cell(slot, a, b, field, sizeof(v), 0u), sizeof(v));
    return (v);
}

void acc_copy_from_shared(void *to, uint32_t from, uint32_t len)
{
    memcpy(to, test_get_table()->shared_base + from, len);
}

void acc_copy_to_shared(uint32_t to, const void *from, uint32_t len)
{
    memcpy(test_get_table()->shared_base + to, from, len);
}
