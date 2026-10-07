/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/wasm/wrong/wrong_declare_more.c -- the wrong module: a
** declaration with no policy, of another size than its descriptor, or made
** in a system; and an allocation by a module that stated none.
*/

#include "wrong.h"

/*
** wrong_declare_policy_unset
**
** Declares a type that does not say what an unload leaves of it.
*/
void wrong_declare_policy_unset(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.on_unload = (enum bridge_on_unload)0;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

/*
** wrong_declare_size_other
**
** Declares a type as twelve bytes, with a descriptor that makes eight.
*/
void wrong_declare_size_other(void)
{
    bridge_declaration_t d = wrong_state_type();

    d.size = 12;
    bridge_declare(bridge_put(&d, sizeof(d)));
}

/*
** wrong_declare_late
**
** Declares a type from a system.
*/
void wrong_declare_late(void)
{
    bridge_declaration_t d = wrong_state_type();

    bridge_declare(bridge_put(&d, sizeof(d)));
}

/*
** wrong_alloc_stated_none
**
** Allocates in the persistent arena.
*/
void wrong_alloc_stated_none(void)
{
    bridge_alloc(8);
}
