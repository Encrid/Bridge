/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_ON_DESTROY_H
    #define TEST_ON_DESTROY_H

    #include "bridge_abi.h"

    #ifdef HUSK
constexpr enum bridge_on_destroy ON_DESTROY = BRIDGE_ON_DESTROY_KEEP;
    #else
constexpr enum bridge_on_destroy ON_DESTROY = BRIDGE_ON_DESTROY_REMOVE;
    #endif

#endif
