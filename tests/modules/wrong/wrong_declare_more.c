/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

void wrong_declare_policy_unset(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.on_unload = (enum bridge_on_unload)0;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_declare_size_other(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.size = 12;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_declare_late(void)
{
    bridge_declaration_t d = wrong_state_type();

    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_alloc_stated_none(void)
{
    bridge_alloc(8);
}
