/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wrong.h -- the wrong module: a module that does one thing
** an entry refuses, at init or in its system. Which thing is stated by
** whoever builds it, as AT_INIT and IN_SYSTEM: a function of this header
** each, and wrong_do_nothing for the place it does no wrong in.
*/

#ifndef TEST_WRONG_H
    #define TEST_WRONG_H

    #include "bridge_wasm.h"
    #include "game.h"

constexpr uint32_t WRONG_OUTSIDE = 0xFFFFFF00u;
constexpr uint32_t WRONG_NO_SLOT = 9;

/*
** The slots init was answered: the position, and the thing of another
** namespace the module is granted to read.
*/
extern uint32_t wrong_position;
extern uint32_t wrong_thing;

/*
** wrong_state_type, wrong_state_system, wrong_state_observer
**
** Answer a declaration, a system and an observer that would be accepted:
** a struct "game:Odd", and "again", which reads the position. A function
** below changes one member and hands it over.
*/
bridge_declaration_t wrong_state_type(void);
bridge_system_statement_t wrong_state_system(void);
bridge_observer_statement_t wrong_state_observer(void);

/*
** wrong_add_with_term, wrong_add_with_change
**
** Register "again" with one term stated as it is handed, and with one
** change of a slot.
*/
void wrong_add_with_term(uint32_t slot, uint8_t access, uint8_t oper);
void wrong_add_with_change(uint32_t slot);

/*
** What is done at init or in the system. Each but the first is refused by
** the entry it calls.
*/
void wrong_do_nothing(void);
void wrong_declare_outside(void);
void wrong_declare_name_outside(void);
void wrong_declare_no_namespace(void);
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

#endif
