/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

void wrong_declare_desc_huge(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.desc_len = 1u << 20;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_declare_again(void)
{
    (void)BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
}

void wrong_declare_align_other(void)
{
    constexpr uint32_t len = sizeof(position_t_desc) - 1;
    bridge_declaration_t d = { bridge_put("game:Position", 13), 13,
        BRIDGE_STRUCT, 8, 8, bridge_put(position_t_desc, len), len,
        BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE };

    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_declare_toggle(void)
{
    (void)BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_STRIP,
        BRIDGE_TOGGLE);
}

void wrong_declare_kind_five(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.kind = (enum bridge_kind)5;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_declare_tag_with_desc(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.kind = BRIDGE_TAG;
    d.size = 0;
    d.align = 1;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_declare_unload_four(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.on_unload = (enum bridge_on_unload)4;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_observe_nine_terms(void)
{
    bridge_term_t terms[9];
    bridge_observer_statement_t o = wrong_state_observer();

    for (uint32_t i = 0; i < 9; i++)
        terms[i] = (bridge_term_t){ wrong_position, BRIDGE_READ, BRIDGE_AND, 0 };
    o.terms = bridge_put(terms, sizeof(terms));
    o.term_count = 9;
    bridge_observe(bridge_put(&o, sizeof(o)));
}

void wrong_system_terms_outside(void)
{
    bridge_system_statement_t s = wrong_state_system();

    s.terms = WRONG_OUTSIDE;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_system_change_outside(void)
{
    bridge_system_statement_t s = wrong_state_system();

    s.changes = WRONG_OUTSIDE;
    s.change_count = 1;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_observe_many_changes(void)
{
    uint32_t changes[5] = { wrong_position, wrong_position, wrong_position,
        wrong_position, wrong_position };
    bridge_observer_statement_t o = wrong_state_observer();

    o.changes = bridge_put(changes, sizeof(changes));
    o.change_count = 5;
    bridge_observe(bridge_put(&o, sizeof(o)));
}

void wrong_observe_twice(void)
{
    bridge_observer_statement_t o = wrong_state_observer();

    bridge_observe(bridge_put(&o, sizeof(o)));
    o = wrong_state_observer();
    bridge_observe(bridge_put(&o, sizeof(o)));
}

static void add_named(const char *name, uint32_t len)
{
    bridge_system_statement_t s = wrong_state_system();

    s.name = bridge_put(name, len);
    s.name_len = len;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_system_named_shutdown(void)
{
    add_named("shutdown", 8);
}

void wrong_system_named_reload(void)
{
    add_named("reload", 6);
}

void wrong_system_ghost(void)
{
    add_named("ghost", 5);
}

void wrong_observe_events_eight(void)
{
    bridge_observer_statement_t o = wrong_state_observer();

    o.events = (enum bridge_event)8;
    bridge_observe(bridge_put(&o, sizeof(o)));
}

void wrong_detach_id_zero(void)
{
    bridge_detach(0, wrong_position);
}

void wrong_declare_near_namespace(void)
{
    static const char desc[] = "{ Flag f; }";
    bridge_declaration_t d = wrong_state_type();
    bridge_declaration_t e = { bridge_put("game:Odd2", 9), 9, BRIDGE_STRUCT,
        1, 1, bridge_put(desc, sizeof(desc) - 1), sizeof(desc) - 1,
        BRIDGE_STRIP, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE };

    d.name = bridge_put("gamex:Odd", 9);
    d.name_len = 9;
    bridge_declare(bridge_put(&d, sizeof(d)));
    bridge_declare(bridge_put(&e, sizeof(e)));
}

BRIDGE_EXPORT(seen) int seen(void)
{
    bridge_attach(1, wrong_flag, BRIDGE_ZEROED);
    return (0);
}

void wrong_observe_entry(void)
{
    bridge_term_t term = { wrong_position, BRIDGE_READ, BRIDGE_AND, 0 };
    bridge_observer_statement_t o = { bridge_put("seen", 4), 4,
        BRIDGE_ON_ENTER, bridge_put(&term, sizeof(term)), 1, 0, 0, 0, 0 };

    bridge_observe(bridge_put(&o, sizeof(o)));
}

void wrong_spawn_position(void)
{
    static bool spawned;
    position_t p = { 0.0f, 0.0f };

    if (spawned)
        return;
    spawned = true;
    BRIDGE_ATTACH(bridge_spawn(), wrong_position, p);
}
