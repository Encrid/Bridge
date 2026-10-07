/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/wrong/wrong_system.c -- the wrong module: a system
** whose statement, phase or name is not in the scratch arena, that states
** more changes than there are types, or whose name is taken.
*/

#include "wrong.h"

/*
** wrong_system_outside
**
** Hands add_system a place that is not in the scratch arena.
*/
void wrong_system_outside(void)
{
    bridge_add_system(WRONG_OUTSIDE);
}

/*
** wrong_system_phase_outside
**
** Registers a system whose phase is not in the scratch arena.
*/
void wrong_system_phase_outside(void)
{
    bridge_system_statement_t s = wrong_state_system();

    s.phase = WRONG_OUTSIDE;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

/*
** wrong_system_name_outside
**
** Registers a system whose name is not in the scratch arena.
*/
void wrong_system_name_outside(void)
{
    bridge_system_statement_t s = wrong_state_system();

    s.name = WRONG_OUTSIDE;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

/*
** wrong_system_many_changes
**
** Registers a system that states five changes: the module declared three
** types.
*/
void wrong_system_many_changes(void)
{
    uint32_t changes[5] = { 0, 0, 0, 0, 0 };
    bridge_system_statement_t s = wrong_state_system();

    s.changes = bridge_put(changes, sizeof(changes));
    s.change_count = 5;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

/*
** wrong_system_twice
**
** Registers a system named as the one init registered.
*/
void wrong_system_twice(void)
{
    bridge_system_statement_t s = wrong_state_system();

    s.name = bridge_put("act", 3);
    s.name_len = 3;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}
