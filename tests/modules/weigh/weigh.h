/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_NWEIGH_H
    #define TEST_NWEIGH_H

    #include "test_module.h"

    #define V_FIELDS { uint32_t v; }
    #define F_FIELDS { uint32_t on; uint32_t v; }
    #define L_FIELDS { uint64_t near; uint64_t far; }
    #define S_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(v_t, V_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(v_t, V_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(f_t, F_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(f_t, F_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(l_t, L_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(l_t, L_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(s_t, S_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(s_t, S_FIELDS);

constexpr uint32_t COUNT = 204'800;
constexpr uint32_t ASKED = 1'000'000;

extern uint32_t nweigh_value;
extern uint32_t nweigh_flag;
extern uint32_t nweigh_link;
extern uint32_t nweigh_single;
extern uint32_t nweigh_walking;
extern uint32_t nweigh_flagged;
extern uint32_t nweigh_linked;
extern volatile uint32_t nweigh_which;

static inline void say_read(const test_api_t *api, uint32_t total,
    uint32_t spans)
{
    bridge_line_t line = { "read", 4, total, spans };

    test_log(api, BRIDGE_LOG_INFO, &line);
}

#endif
