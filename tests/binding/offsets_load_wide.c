/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/binding/offsets_load_wide.c -- the shared memory read, by width:
** the wide ones, and bytes copied out of it and into it.
**
** abi.md, *The ABI*
*/

#include <string.h>

#include "test_offsets_acc.h"

/*
** acc_ld_64
**
** Reads 64 bits of the shared memory.
*/
uint64_t acc_ld_64(uint32_t at)
{
    uint64_t v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}

/*
** acc_ld_f32
**
** Reads a float of the shared memory.
*/
float acc_ld_f32(uint32_t at)
{
    float v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}

/*
** acc_ld_f64
**
** Reads a double of the shared memory.
*/
double acc_ld_f64(uint32_t at)
{
    double v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}

/*
** acc_from_shared
**
** Copies bytes out of the shared memory.
*/
void acc_from_shared(void *to, uint32_t from, uint32_t len)
{
    memcpy(to, test_get_table()->shared_base + from, len);
}

/*
** acc_to_shared
**
** Copies bytes into the shared memory.
*/
void acc_to_shared(uint32_t to, const void *from, uint32_t len)
{
    memcpy(test_get_table()->shared_base + to, from, len);
}
