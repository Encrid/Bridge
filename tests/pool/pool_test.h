/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_TESTS_POOL_TEST_H
    #define BRIDGE_TESTS_POOL_TEST_H

    #include "pool.h"

constexpr size_t POOL_TEST_STACK = 1 << 18;

int pool_test_meet(uint32_t workers);

#endif
