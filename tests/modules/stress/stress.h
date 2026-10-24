/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_NSTRESS_H
    #define TEST_NSTRESS_H

    #include "test_module.h"

    #define V_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(v_t, V_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(v_t, V_FIELDS);

constexpr uint32_t KINDS = 8;
constexpr uint32_t SLOTS = 50'000;
constexpr uint32_t CHANGES = 20'000;

extern uint32_t nstress_kinds[KINDS];
extern uint32_t nstress_seeing[KINDS];

extern uint64_t nstress_ids[SLOTS];
extern uint8_t nstress_has[SLOTS];
extern uint32_t nstress_values[SLOTS][KINDS];
extern uint32_t nstress_wrong;

void nstress_check_entities(const test_api_t *api);

int nstress_check_kind(const test_api_t *api, uint32_t kind);

#endif
