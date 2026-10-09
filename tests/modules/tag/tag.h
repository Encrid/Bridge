/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/tag/tag.h -- what the sources of ntag share: the
** one component that has bytes, what init was answered, and the entities.
*/

#ifndef TEST_NTAG_H
    #define TEST_NTAG_H

    #include "test_module.h"
    #include "say.h"

    #define A_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(a_t, A_FIELDS);

constexpr uint32_t COUNT = 6;

/*
** The slots of A and of the five tags, and the queries of the three systems
** that walk: `look`, `in_set` and `flicker`.
*/
extern uint32_t ntag_a;
extern uint32_t ntag_m;
extern uint32_t ntag_p;
extern uint32_t ntag_o;
extern uint32_t ntag_r;
extern uint32_t ntag_w;
extern uint32_t ntag_marked;
extern uint32_t ntag_in_set;
extern uint32_t ntag_toggled;
extern uint64_t ntag_ids[COUNT];

/*
** ntag_look_set
**
** Says what a module is handed for P, the tag kept in a sparse set: by the
** spans of a query on it alone, and by each entity's cell of its index.
*/
void ntag_look_set(const test_api_t *api);

#endif
