/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/wrong/wrong_detach.c -- the wrong module: a detach
** an entry refuses.
*/

#include "wrong.h"

/*
** wrong_detach_undeclared
**
** Detaches a slot the module never declared.
*/
void wrong_detach_undeclared(void)
{
    bridge_detach(1, WRONG_NO_SLOT);
}
