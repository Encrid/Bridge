/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

void wrong_declare_outside(void)
{
    bridge_declare(WRONG_OUTSIDE);
}

void wrong_declare_name_outside(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.name = WRONG_OUTSIDE;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_declare_no_namespace(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.name = bridge_put("Odd", 3);
    d.name_len = 3;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_declare_name_zero(void)
{
    static const char name[5] = { 'a', ':', 'b', 0, 'c' };
    bridge_declaration_t d = wrong_state_type();

    d.name = bridge_put(name, 5);
    d.name_len = 5;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_declare_kind_unset(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.kind = (enum bridge_kind)0;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_declare_tag_with_bytes(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.kind = BRIDGE_TAG;
    bridge_declare(bridge_put(&d, sizeof(d)));
}
