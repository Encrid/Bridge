/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

void wrong_observer_no_term(void)
{
    bridge_observer_statement_t o = wrong_state_observer();

    o.term_count = 0;
    bridge_observe(bridge_put(&o, sizeof(o)));
}

void wrong_observe_outside(void)
{
    bridge_observe(WRONG_OUTSIDE);
}

void wrong_observe_events_unset(void)
{
    bridge_observer_statement_t o = wrong_state_observer();

    o.events = (enum bridge_event)0;
    bridge_observe(bridge_put(&o, sizeof(o)));
}

void wrong_system_late(void)
{
    bridge_system_statement_t s = wrong_state_system();

    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_observe_late(void)
{
    bridge_observer_statement_t o = wrong_state_observer();

    bridge_observe(bridge_put(&o, sizeof(o)));
}
