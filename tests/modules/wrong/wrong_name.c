/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

static uint32_t put_long_name(void)
{
    char name[128];
    uint32_t i = 0;

    while (i < sizeof(name)) {
        name[i] = 'o';
        i++;
    }
    return (bridge_put(name, sizeof(name)));
}

void wrong_declare_name_empty(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.name_len = 0;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_declare_name_long(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.name = put_long_name();
    d.name_len = 128;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

void wrong_system_name_empty(void)
{
    bridge_system_statement_t s = wrong_state_system();

    s.name_len = 0;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}

void wrong_system_name_long(void)
{
    bridge_system_statement_t s = wrong_state_system();

    s.name = put_long_name();
    s.name_len = 128;
    bridge_add_system(bridge_put(&s, sizeof(s)));
}
