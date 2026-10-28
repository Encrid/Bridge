/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

void wrong_declare_on_destroy_unset(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.on_destroy = (enum bridge_on_destroy)0;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_declare_on_destroy_three(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.on_destroy = (enum bridge_on_destroy)3;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_declare_singleton_keep(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.storage = BRIDGE_SINGLETON;
    d.on_destroy = BRIDGE_ON_DESTROY_KEEP;
    bridge_declare(bridge_put(&d, sizeof(d)));
}
