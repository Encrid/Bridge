/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_HUSK_H
    #define TEST_HUSK_H

    #include "test_offsets.h"
    #include "on_destroy.h"

    #define BODY_FIELDS { uint32_t v; }
    #define HANDLE_FIELDS { uint32_t gpu; }

BRIDGE_SCHEMA_STRUCT_TYPE(body_t, BODY_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(body_t, BODY_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(handle_t, HANDLE_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(handle_t, HANDLE_FIELDS);

constexpr uint32_t HUSK_DONE = 100;

extern uint32_t husk_body;
extern uint32_t husk_handle;
extern uint32_t husk_finding;
extern uint32_t husk_on_body;
extern uint32_t husk_on_handle;
extern uint32_t husk_on_lone;
extern uint32_t husk_frame;
extern uint32_t husk_heard[3];
extern uint64_t husk_ids[3];

uint32_t husk_get_value(uint64_t id, uint32_t slot);

#endif
