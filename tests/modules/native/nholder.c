/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/nholder.c -- declares a struct with a field of
** st:vec_t, a type another module declares: its rows hold a vec_t as it is,
** so no reload of that other module may lay vec_t out again.
**
** lifetime.md, *When a component layout changed*
*/

#include "bridge_native.h"
#include "native_idle.h"
#include "nholder.h"

/*
** init
**
** Declares st:Y, whose one field is of the other module's type.
*/
BRIDGE_NATIVE(init)
{
    BRIDGE_NATIVE_COMPONENT(api, "st:Y", y_t, BRIDGE_DELETE, BRIDGE_TABLE);
    return (0);
}
