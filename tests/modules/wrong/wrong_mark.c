/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

void wrong_system_name_at(void)
{
    bridge_system_statement_t s = wrong_state_system();

    s.name = bridge_put("fire@will", 9);
    s.name_len = 9;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_observer_name_colon(void)
{
    bridge_observer_statement_t o = wrong_state_observer();

    o.name = bridge_put("on:enter", 8);
    o.name_len = 8;
    bridge_observe(bridge_put(&o, sizeof(o)));
}
