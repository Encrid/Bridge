/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "idle.h"

static bool guessed;

TEST(guess)
{
    if (!guessed)
        api->destroy(api, (uint64_t)1 << 32 | TARGET);
    guessed = true;
    return (0);
}

TEST(init)
{
    int none = 0;

    TEST_ADD_SYSTEM(api, "guess", "update", &none, 0, nullptr, 0);
    return (0);
}
