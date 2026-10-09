/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/store/store_heard_both.c -- what the observers of
** two components hear, in the plain build of nstore.
*/

#include "store.h"

#if !defined(BIG) && !defined(TABLES)
uint32_t nstore_on_both;
uint32_t nstore_on_alone;

/*
** heard_both
**
** Says what the observer of A and B heard.
*/
TEST(heard_both)
{
    return (nstore_say_heard(api, "A and B: who, events", nstore_on_both));
}

/*
** heard_alone
**
** Says what the observer of A and not B heard.
*/
TEST(heard_alone)
{
    return (nstore_say_heard(api, "A and not B: who, events",
        nstore_on_alone));
}

#endif
