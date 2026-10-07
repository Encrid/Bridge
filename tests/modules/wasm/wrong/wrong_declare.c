/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/wrong/wrong_declare.c -- the wrong module: a
** declaration that is not in the scratch arena, or has no namespace, no
** kind, or a tag's bytes.
*/

#include "wrong.h"

/*
** wrong_declare_outside
**
** Hands declare a place that is not in the scratch arena.
*/
void wrong_declare_outside(void)
{
    bridge_declare(WRONG_OUTSIDE);
}

/*
** wrong_declare_name_outside
**
** Declares a type whose name is not in the scratch arena.
*/
void wrong_declare_name_outside(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.name = WRONG_OUTSIDE;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

/*
** wrong_declare_no_namespace
**
** Declares a type whose name has no colon.
*/
void wrong_declare_no_namespace(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.name = bridge_put("Odd", 3);
    d.name_len = 3;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

/*
** wrong_declare_kind_unset
**
** Declares a type of no kind.
*/
void wrong_declare_kind_unset(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.kind = (enum bridge_kind)0;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

/*
** wrong_declare_tag_with_bytes
**
** Declares a tag that states bytes and a descriptor.
*/
void wrong_declare_tag_with_bytes(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.kind = BRIDGE_TAG;
    bridge_declare(bridge_put(&d, sizeof(d)));
}
