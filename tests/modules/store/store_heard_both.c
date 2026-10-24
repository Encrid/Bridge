/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "store.h"

#if !defined(BIG) && !defined(TABLES)
uint32_t nstore_on_both;
uint32_t nstore_on_alone;

TEST(heard_both)
{
    return (nstore_say_heard(api, "A and B: who, events", nstore_on_both));
}

TEST(heard_alone)
{
    return (nstore_say_heard(api, "A and not B: who, events",
        nstore_on_alone));
}

#endif
