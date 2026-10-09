/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/binding/offsets_store.c -- the shared memory written, by width.
**
** abi.md, *The ABI*
*/

#include <string.h>

#include "test_offsets_acc.h"

/*
** acc_st_8
**
** Writes a byte of the shared memory.
*/
void acc_st_8(uint32_t at, uint32_t v)
{
    uint8_t narrow = (uint8_t)v;
    uint8_t *to = test_get_table()->shared_base + at;

    memcpy(to, &narrow, sizeof(narrow));
}

/*
** acc_st_16
**
** Writes 16 bits of the shared memory.
*/
void acc_st_16(uint32_t at, uint32_t v)
{
    uint16_t narrow = (uint16_t)v;
    uint8_t *to = test_get_table()->shared_base + at;

    memcpy(to, &narrow, sizeof(narrow));
}

/*
** acc_st_32
**
** Writes 32 bits of the shared memory.
*/
void acc_st_32(uint32_t at, uint32_t v)
{
    memcpy(test_get_table()->shared_base + at, &v, sizeof(v));
}

/*
** acc_st_64
**
** Writes 64 bits of the shared memory.
*/
void acc_st_64(uint32_t at, uint64_t v)
{
    memcpy(test_get_table()->shared_base + at, &v, sizeof(v));
}

/*
** acc_st_f32
**
** Writes a float of the shared memory.
*/
void acc_st_f32(uint32_t at, float v)
{
    memcpy(test_get_table()->shared_base + at, &v, sizeof(v));
}
