/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

void wrong_system_before_own(void)
{
    uint32_t own = 1;
    bridge_system_statement_t s = wrong_state_system();

    s.before = bridge_put(&own, sizeof(own));
    s.before_count = 1;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_system_before_other_phase(void)
{
    uint32_t act = 0;
    bridge_system_statement_t s = wrong_state_system();

    s.phase = bridge_put("present", 7);
    s.phase_len = 7;
    s.before = bridge_put(&act, sizeof(act));
    s.before_count = 1;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_system_before_outside(void)
{
    bridge_system_statement_t s = wrong_state_system();

    s.before = WRONG_OUTSIDE;
    s.before_count = 1;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_observe_before_own(void)
{
    uint32_t own = 1;
    bridge_observer_statement_t o = wrong_state_observer();

    wrong_observe_entry();
    o.before = bridge_put(&own, sizeof(own));
    o.before_count = 1;
    bridge_observe(bridge_put(&o, sizeof(o)));
}

void wrong_observe_before_outside(void)
{
    bridge_observer_statement_t o = wrong_state_observer();

    o.before = WRONG_OUTSIDE;
    o.before_count = 1;
    bridge_observe(bridge_put(&o, sizeof(o)));
}
