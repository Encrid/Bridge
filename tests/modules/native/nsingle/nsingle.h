/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nsingle/nsingle.h -- what the sources of nsingle
** share: the singleton's type and a table's, and what init was answered.
*/

#ifndef TEST_NSINGLE_H
    #define TEST_NSINGLE_H

    #include "bridge_native.h"
    #include "native/native_say.h"

    #define G_FIELDS { uint32_t gravity; uint32_t wind; }
    #define A_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(g_t, G_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(g_t, G_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(a_t, A_FIELDS);

constexpr uint32_t ASKED = 1'000'000;

/*
** The slots of G, the singleton, and of A, which is kept in a table; the
** observer of G; and which singleton `one` reads, behind a load the
** compiler cannot see through.
*/
extern uint32_t nsingle_g;
extern uint32_t nsingle_a;
extern uint32_t nsingle_on_g;
extern volatile uint32_t nsingle_which;

#endif
