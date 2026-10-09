/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/binding/offsets_store_more.c -- a double written to the shared
** memory, and the private memory written.
**
** abi.md, *The ABI*
*/

#include <string.h>

#include "test_offsets_acc.h"

/*
** acc_st_f64
**
** Writes a double of the shared memory.
*/
void acc_st_f64(uint32_t at, double v)
{
    memcpy(test_get_table()->shared_base + at, &v, sizeof(v));
}

/*
** acc_own_st
**
** Writes 32 bits of the private memory.
*/
void acc_own_st(uint32_t at, uint32_t v)
{
    memcpy(test_get_table()->private_base + at, &v, sizeof(v));
}

/*
** acc_to_private
**
** Copies bytes into the private memory.
*/
void acc_to_private(uint32_t to, const void *from, uint32_t len)
{
    memcpy(test_get_table()->private_base + to, from, len);
}
