/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_NSPARSE_H
    #define TEST_NSPARSE_H

    #include "test_module.h"
    #include "say.h"

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

extern uint32_t nsparse_a;
extern uint32_t nsparse_s;
extern uint32_t nsparse_in_set;
extern uint32_t nsparse_in_table;
extern uint64_t nsparse_ids[COUNT];
extern uint64_t nsparse_lone;

    #if !defined(BIG) && !defined(FAT)
extern uint32_t nsparse_on_s;
extern uint32_t nsparse_on_both;
    #endif

#endif
