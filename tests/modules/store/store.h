/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_NSTORE_H
    #define TEST_NSTORE_H

    #include "test_module.h"
    #include "say.h"

    #if defined(FAT64)
        #define A_FIELDS { uint32_t v; char rest[60]; }
    #elif defined(FAT256)
        #define A_FIELDS { uint32_t v; char rest[252]; }
    #elif defined(FAT1K)
        #define A_FIELDS { uint32_t v; char rest[1020]; }
    #elif defined(FAT4K)
        #define A_FIELDS { uint32_t v; char rest[4092]; }
    #else
        #define A_FIELDS { uint32_t v; }
    #endif
    #define B_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(b_t, B_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(b_t, B_FIELDS);

    #if defined(FAT1K) || defined(FAT4K)
constexpr uint32_t COUNT = 10'000;
    #elifdef BIG
constexpr uint32_t COUNT = 100'000;
    #elifdef TABLES
constexpr uint32_t COUNT = 10'000;
    #else
constexpr uint32_t COUNT = 6;
    #endif

extern uint32_t nstore_a;
extern uint32_t nstore_b;
extern uint64_t nstore_ids[COUNT];

    #ifdef TABLES
        #define T_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(t_t, T_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(t_t, T_FIELDS);

constexpr uint32_t KINDS = 14;

void nstore_add_tables(const test_api_t *api);
    #endif

    #if !defined(BIG) && !defined(TABLES)
extern uint32_t nstore_with_a;
extern uint32_t nstore_without_b;
extern uint32_t nstore_maybe_b;
extern uint32_t nstore_with_b;
extern uint32_t nstore_on_a;
extern uint32_t nstore_on_b;
extern uint32_t nstore_on_both;
extern uint32_t nstore_on_alone;
extern uint64_t nstore_old[COUNT];

void nstore_add_plain(const test_api_t *api);

int nstore_say_heard(const test_api_t *api, const char *what,
    uint32_t observer);
    #endif

#endif
