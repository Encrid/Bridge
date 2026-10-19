/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

void wrong_system_outside(void)
{
    bridge_add_system(WRONG_OUTSIDE);
}

void wrong_system_phase_outside(void)
{
    bridge_system_statement_t s = wrong_state_system();

    s.phase = WRONG_OUTSIDE;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_system_name_outside(void)
{
    bridge_system_statement_t s = wrong_state_system();

    s.name = WRONG_OUTSIDE;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_system_many_changes(void)
{
    uint32_t changes[5] = { 0, 0, 0, 0, 0 };
    bridge_system_statement_t s = wrong_state_system();

    s.changes = bridge_put(changes, sizeof(changes));
    s.change_count = 5;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_system_twice(void)
{
    bridge_system_statement_t s = wrong_state_system();

    s.name = bridge_put("act", 3);
    s.name_len = 3;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}
