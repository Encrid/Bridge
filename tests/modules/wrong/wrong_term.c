/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wrong/wrong_term.c -- the wrong module: a term with
** no access, an access there is not, no operator, a slot nobody declared,
** or a write no grant allows.
*/

#include "wrong.h"

/*
** wrong_term_access_unset
**
** Registers a system whose term states no access.
*/
void wrong_term_access_unset(void)
{
    wrong_add_with_term(wrong_position, 0, BRIDGE_AND);
}

/*
** wrong_term_access_other
**
** Registers a system whose term states an access that is neither.
*/
void wrong_term_access_other(void)
{
    wrong_add_with_term(wrong_position, 3, BRIDGE_AND);
}

/*
** wrong_term_oper_unset
**
** Registers a system whose term states no operator.
*/
void wrong_term_oper_unset(void)
{
    wrong_add_with_term(wrong_position, BRIDGE_READ, 0);
}

/*
** wrong_term_slot_undeclared
**
** Registers a system whose term names a slot the module never declared.
*/
void wrong_term_slot_undeclared(void)
{
    wrong_add_with_term(WRONG_NO_SLOT, BRIDGE_READ, BRIDGE_AND);
}

/*
** wrong_term_write_ungranted
**
** Registers a system that writes what the module is granted to read.
*/
void wrong_term_write_ungranted(void)
{
    wrong_add_with_term(wrong_thing, BRIDGE_WRITE, BRIDGE_AND);
}
