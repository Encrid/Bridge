/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_NTAG_H
    #define TEST_NTAG_H

    #include "test_module.h"
    #include "say.h"

    #define A_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(a_t, A_FIELDS);

constexpr uint32_t COUNT = 6;

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

void ntag_look_set(const test_api_t *api);

#endif
