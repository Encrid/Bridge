/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/wasm/wrong/wrong_observe.c -- the wrong module: an
** observer with no term or no event, one whose statement is not in the
** scratch arena, and a system or an observer registered from a system.
*/

#include "wrong.h"

/*
** wrong_observer_no_term
**
** Registers an observer that has no term.
*/
void wrong_observer_no_term(void)
{
    bridge_observer_statement_t o = wrong_state_observer();

    o.term_count = 0;
    bridge_observe(bridge_put(&o, sizeof(o)));
}

/*
** wrong_observe_outside
**
** Hands observe a place that is not in the scratch arena.
*/
void wrong_observe_outside(void)
{
    bridge_observe(WRONG_OUTSIDE);
}

/*
** wrong_observe_events_unset
**
** Registers an observer of no event.
*/
void wrong_observe_events_unset(void)
{
    bridge_observer_statement_t o = wrong_state_observer();

    o.events = (enum bridge_event)0;
    bridge_observe(bridge_put(&o, sizeof(o)));
}

/*
** wrong_system_late
**
** Registers a system from a system.
*/
void wrong_system_late(void)
{
    bridge_system_statement_t s = wrong_state_system();

    bridge_add_system(bridge_put(&s, sizeof(s)));
}

/*
** wrong_observe_late
**
** Registers an observer from a system.
*/
void wrong_observe_late(void)
{
    bridge_observer_statement_t o = wrong_state_observer();

    bridge_observe(bridge_put(&o, sizeof(o)));
}
