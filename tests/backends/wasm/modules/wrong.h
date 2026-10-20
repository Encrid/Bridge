/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_WRONG_H
    #define TEST_WRONG_H

    #include "bridge_wasm.h"
    #include "game.h"

constexpr uint32_t WRONG_OUTSIDE = 0xFFFFFF00u;
constexpr uint32_t WRONG_NO_SLOT = 9;

extern uint32_t wrong_position;
extern uint32_t wrong_thing;
extern uint32_t wrong_flag;

bridge_declaration_t wrong_state_type(void);
bridge_system_statement_t wrong_state_system(void);
bridge_observer_statement_t wrong_state_observer(void);

void wrong_add_with_term(uint32_t slot, uint8_t access, uint8_t oper);
void wrong_add_with_change(uint32_t slot);

void wrong_do_nothing(void);
void wrong_declare_outside(void);
void wrong_declare_name_outside(void);
void wrong_declare_no_namespace(void);
void wrong_declare_name_zero(void);
void wrong_declare_kind_unset(void);
void wrong_declare_tag_with_bytes(void);
void wrong_declare_policy_unset(void);
void wrong_declare_size_other(void);
void wrong_declare_late(void);
void wrong_system_outside(void);
void wrong_system_phase_outside(void);
void wrong_system_name_outside(void);
void wrong_system_many_changes(void);
void wrong_system_twice(void);
void wrong_system_late(void);
void wrong_observer_no_term(void);
void wrong_observe_outside(void);
void wrong_observe_events_unset(void);
void wrong_observe_late(void);
void wrong_term_access_unset(void);
void wrong_term_access_other(void);
void wrong_term_oper_unset(void);
void wrong_term_slot_undeclared(void);
void wrong_term_write_ungranted(void);
void wrong_change_slot_undeclared(void);
void wrong_change_ungranted(void);
void wrong_alloc_stated_none(void);
void wrong_attach_undeclared(void);
void wrong_attach_ungranted(void);
void wrong_attach_outside(void);
void wrong_detach_undeclared(void);
void wrong_attach_tag_value(void);

#endif
