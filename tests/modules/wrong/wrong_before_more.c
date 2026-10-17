/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

void wrong_system_before_many(void)
{
    uint32_t act[2] = { 0, 0 };
    bridge_system_statement_t s = wrong_state_system();

    s.before = bridge_put(act, sizeof(act));
    s.before_count = 2;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_system_before_same_phase(void)
{
    uint32_t act = 0;
    bridge_system_statement_t s = wrong_state_system();

    s.before = bridge_put(&act, sizeof(act));
    s.before_count = 1;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_observe_before_earlier(void)
{
    uint32_t seen = 0;
    bridge_observer_statement_t o = wrong_state_observer();

    wrong_observe_entry();
    o.before = bridge_put(&seen, sizeof(seen));
    o.before_count = 1;
    bridge_observe(bridge_put(&o, sizeof(o)));
}

void wrong_observe_before_many(void)
{
    uint32_t first = 0;
    bridge_observer_statement_t o = wrong_state_observer();

    o.before = bridge_put(&first, sizeof(first));
    o.before_count = 1;
    bridge_observe(bridge_put(&o, sizeof(o)));
}
