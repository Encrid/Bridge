/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

void wrong_term_access_unset(void)
{
    wrong_add_with_term(wrong_position, 0, BRIDGE_AND);
}

void wrong_term_access_other(void)
{
    wrong_add_with_term(wrong_position, 3, BRIDGE_AND);
}

void wrong_term_oper_unset(void)
{
    wrong_add_with_term(wrong_position, BRIDGE_READ, 0);
}

void wrong_term_slot_undeclared(void)
{
    wrong_add_with_term(WRONG_NO_SLOT, BRIDGE_READ, BRIDGE_AND);
}

void wrong_term_write_ungranted(void)
{
    wrong_add_with_term(wrong_thing, BRIDGE_WRITE, BRIDGE_AND);
}
