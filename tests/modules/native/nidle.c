/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/native/nidle.c -- a shutdown and a reload that do nothing,
** built into every native test module that includes native_idle.h.
*/

#include "native_idle.h"

/*
** shutdown
**
** Does nothing.
*/
BRIDGE_NATIVE(shutdown)
{
    return (0);
}

/*
** reload
**
** Does nothing.
*/
BRIDGE_NATIVE(reload)
{
    return (0);
}
