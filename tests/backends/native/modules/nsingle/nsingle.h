/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_NSINGLE_H
    #define TEST_NSINGLE_H

    #include "bridge_native.h"
    #include "native_say.h"

    #define G_FIELDS { uint32_t gravity; uint32_t wind; }
    #define A_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(g_t, G_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(g_t, G_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(a_t, A_FIELDS);

constexpr uint32_t ASKED = 1'000'000;

extern uint32_t nsingle_g;
extern uint32_t nsingle_a;
extern uint32_t nsingle_on_g;
extern volatile uint32_t nsingle_which;

#endif
