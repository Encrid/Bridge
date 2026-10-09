/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/binding/offsets_load.c -- the shared memory read, by width: the
** narrow ones.
**
** abi.md, *The ABI*
*/

#include <string.h>

#include "test_offsets_acc.h"

/*
** acc_ld_i8
**
** Reads a signed byte of the shared memory.
*/
int32_t acc_ld_i8(uint32_t at)
{
    int8_t v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}

/*
** acc_ld_u8
**
** Reads a byte of the shared memory.
*/
uint32_t acc_ld_u8(uint32_t at)
{
    uint8_t v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}

/*
** acc_ld_i16
**
** Reads 16 signed bits of the shared memory.
*/
int32_t acc_ld_i16(uint32_t at)
{
    int16_t v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}

/*
** acc_ld_u16
**
** Reads 16 bits of the shared memory.
*/
uint32_t acc_ld_u16(uint32_t at)
{
    uint16_t v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}

/*
** acc_ld_32
**
** Reads 32 bits of the shared memory.
*/
uint32_t acc_ld_32(uint32_t at)
{
    uint32_t v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}
