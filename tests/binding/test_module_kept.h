/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/binding/test_module_kept.h -- what is kept in other layouts than
** a table, read by a module of the tests.
**
** storage.md, *A sparse set*
*/

#ifndef TEST_MODULE_KEPT_H
    #define TEST_MODULE_KEPT_H

    #include "test_module_api.h"

/*
** test_get_single
**
** Answers the value of the singleton `slot`, or nullptr while it has none.
** It is given by an attach with the id 0, taken by a detach with it, and
** written where it is by a system that stated the term BRIDGE_WRITE.
**
** storage.md, *A singleton*
*/
static inline void *test_get_single(const test_api_t *api,
    uint32_t slot)
{
    const uint8_t *kept = api->private_base
        + test_get_header(api)->kept;
    uint32_t at = ((const uint32_t *)kept)[slot];

    return (at ? api->shared_base + at : nullptr);
}

/*
** test_get_cells
**
** Answers the cells of the direct-indexed component `slot`, from the shared
** base: the value of the entity whose id's low 32 bits are e is cell e.
** Whether that entity has been given one is its bit, bit e & 63 of word
** e >> 6 of test_get_bits of the same offset.
**
** storage.md, *Direct-indexed*
*/
static inline uint32_t test_get_cells(const test_api_t *api,
    uint32_t slot)
{
    const uint8_t *kept = api->private_base
        + test_get_header(api)->kept;

    return (((const uint32_t *)kept)[slot]);
}

/*
** test_get_sparse_index
**
** Answers the index of the sparse set that keeps component `slot`: a cell
** for each entity, by the low 32 bits of its id, holding where that
** entity's value is from the shared base, and zero where it has none.
**
** storage.md, *A sparse set*
*/
static inline const uint32_t *test_get_sparse_index(
    const test_api_t *api, uint32_t slot)
{
    const uint8_t *kept = api->private_base
        + test_get_header(api)->kept;

    return ((const uint32_t *)(api->shared_base
        + ((const uint32_t *)kept)[slot]));
}

/*
** test_get_bits
**
** Answers the bits of the toggled column that begins at `base`, which a
** span or column_base gave: row r is bit r & 63 of word r >> 6, and a row
** whose bit is 0 is off. A system that stated the term BRIDGE_WRITE turns a
** row on or off by writing its bit.
**
** storage.md, *A toggle*
*/
static inline uint64_t *test_get_bits(const test_api_t *api,
    uint32_t base)
{
    uint32_t at;

    memcpy(&at, api->shared_base + base - 16, 4);
    return ((uint64_t *)(api->shared_base + at));
}

/*
** test_get_list
**
** Answers an observer's list: `count` bridge_trigger_t records, at
** private_base + `at`.
**
** data.md, *Observers*
*/
static inline const bridge_list_t *test_get_list(
    const test_api_t *api, uint32_t observer)
{
    const uint8_t *lists = api->private_base
        + test_get_header(api)->observers;

    return ((const bridge_list_t *)lists + observer);
}

#endif
