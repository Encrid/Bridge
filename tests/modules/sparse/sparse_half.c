/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/sparse/sparse_half.c -- the systems of nsparse
** that change one entity or two.
*/

#include "sparse.h"

uint64_t nsparse_lone;

/*
** detach_b_half
**
** Takes S from the first entity that has it.
*/
TEST(detach_b_half)
{
    api->detach(api, nsparse_ids[0], nsparse_s);
    return (0);
}

/*
** value_b
**
** Gives a value to an entity that has S already.
*/
TEST(value_b)
{
    s_t value = { 900 };

    TEST_ATTACH(api, nsparse_ids[2 % COUNT], nsparse_s, value);
    return (0);
}

/*
** attach_b_half
**
** Spawns one more entity, with S and nothing else: it is in no table.
*/
TEST(attach_b_half)
{
    s_t value = { 7 };

    nsparse_lone = api->spawn(api);
    TEST_ATTACH(api, nsparse_lone, nsparse_s, value);
    return (0);
}

/*
** destroy_half
**
** Destroys an entity that has S, and one that has not.
*/
TEST(destroy_half)
{
    api->destroy(api, nsparse_ids[2 % COUNT]);
    api->destroy(api, nsparse_ids[1 % COUNT]);
    return (0);
}
