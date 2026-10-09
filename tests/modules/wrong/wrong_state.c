/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wrong/wrong_state.c -- the wrong module: a
** declaration, a system and an observer that would each be accepted, for
** a function to change one member of.
*/

#include "wrong.h"

/*
** wrong_state_type
**
** Answers the declaration of a struct "game:Odd", laid out as a position.
*/
bridge_declaration_t wrong_state_type(void)
{
    constexpr uint32_t len = sizeof(position_t_desc) - 1;
    bridge_declaration_t d = { bridge_put("game:Odd", 8), 8, BRIDGE_STRUCT,
        sizeof(position_t), alignof(position_t),
        bridge_put(position_t_desc, len), len, BRIDGE_STRIP, BRIDGE_TABLE };

    return (d);
}

/*
** wrong_state_system
**
** Answers the statement of a system "again" in "update", which reads the
** position and changes nothing.
*/
bridge_system_statement_t wrong_state_system(void)
{
    bridge_term_t term = { wrong_position, BRIDGE_READ, BRIDGE_AND, 0 };
    bridge_system_statement_t s = { bridge_put("again", 5), 5,
        bridge_put("update", 6), 6, bridge_put(&term, sizeof(term)), 1, 0,
        0 };

    return (s);
}

/*
** wrong_state_observer
**
** Answers the statement of an observer "again" of the position entering.
*/
bridge_observer_statement_t wrong_state_observer(void)
{
    bridge_term_t term = { wrong_position, BRIDGE_READ, BRIDGE_AND, 0 };
    bridge_observer_statement_t o = { bridge_put("again", 5), 5,
        BRIDGE_ON_ENTER, bridge_put(&term, sizeof(term)), 1, 0, 0 };

    return (o);
}

/*
** wrong_add_with_term
**
** Registers "again" with one term, stated as it is handed.
*/
void wrong_add_with_term(uint32_t slot, uint8_t access, uint8_t oper)
{
    bridge_term_t term = { slot, (enum bridge_access)access,
        (enum bridge_oper)oper, 0 };
    bridge_system_statement_t s = wrong_state_system();

    s.terms = bridge_put(&term, sizeof(term));
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

/*
** wrong_add_with_change
**
** Registers "again" with one change, of a slot.
*/
void wrong_add_with_change(uint32_t slot)
{
    bridge_system_statement_t s = wrong_state_system();

    s.changes = bridge_put(&slot, sizeof(slot));
    s.change_count = 1;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}
