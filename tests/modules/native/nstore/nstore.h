/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/nstore/nstore.h -- what the sources of nstore share:
** its components, how many entities the build has, and what init was
** answered.
*/

#ifndef TEST_NSTORE_H
    #define TEST_NSTORE_H

    #include "bridge_native.h"
    #include "native/native_say.h"

/*
** A is as big as the build says: for what a change costs an entity that
** carries more. The length is written out each time, since a descriptor is
** the text as it was typed.
*/
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

/*
** The slots of A and of B, and the entities.
*/
extern uint32_t nstore_a;
extern uint32_t nstore_b;
extern uint64_t nstore_ids[COUNT];

/*
** The build TABLES: fourteen more components, each of this type. There are
** 2^14 sets of them, and each set is a table.
*/
    #ifdef TABLES
        #define T_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(t_t, T_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(t_t, T_FIELDS);

constexpr uint32_t KINDS = 14;

/*
** nstore_add_tables
**
** Declares the fourteen components, and registers the system that gives
** each entity a set of them of its own and the system that sees the first.
*/
void nstore_add_tables(const bridge_api_t *api);
    #endif

/*
** The plain build, which is neither BIG nor TABLES: the queries of `look`,
** `look_not`, `look_maybe` and `look_b`; the four observers; and the ids of
** the entities `destroy_half` destroyed, where they were.
*/
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

/*
** nstore_add_plain
**
** Registers the systems and the observers that only the plain build has.
*/
void nstore_add_plain(const bridge_api_t *api);

/*
** nstore_say_heard
**
** Says each thing `observer` heard, under the text `what`: who, and the
** events. Who is where the id sits among the module's own, so that nothing
** said depends on how ids are numbered. Answers 0.
*/
int nstore_say_heard(const bridge_api_t *api, const char *what,
    uint32_t observer);
    #endif

#endif
