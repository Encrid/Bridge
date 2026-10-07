/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/native/nsparse/nsparse.h -- what the sources of nsparse
** share: the two components, how many entities the build has, and what
** init was answered.
*/

#ifndef TEST_NSPARSE_H
    #define TEST_NSPARSE_H

    #include "bridge_native.h"
    #include "native/native_say.h"

    #ifdef FAT
        #define A_FIELDS { uint32_t v; uint8_t rest[4092]; }
    #else
        #define A_FIELDS { uint32_t v; }
    #endif
    #define S_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(s_t, S_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(s_t, S_FIELDS);

/*
** How many entities there are, and one in how many of them is given S.
*/
    #ifdef FAT
constexpr uint32_t COUNT = 10'000;
constexpr uint32_t STEP = 1;
    #elifdef BIG
constexpr uint32_t COUNT = 204'800;
constexpr uint32_t STEP = 1;
    #else
constexpr uint32_t COUNT = 6;
constexpr uint32_t STEP = 2;
    #endif

/*
** The slots of A and of S, the queries of `look` and of `look_b`, the
** entities, and the one more that has S and nothing else.
*/
extern uint32_t nsparse_a;
extern uint32_t nsparse_s;
extern uint32_t nsparse_in_set;
extern uint32_t nsparse_in_table;
extern uint64_t nsparse_ids[COUNT];
extern uint64_t nsparse_lone;

/*
** The observer of S, and the observer of A and S. A build for a measurement
** has no observer.
*/
    #if !defined(BIG) && !defined(FAT)
extern uint32_t nsparse_on_s;
extern uint32_t nsparse_on_both;
    #endif

#endif
