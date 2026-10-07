/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/wrong/wrong_detach.c -- the wrong module: a detach
** an entry refuses, and a tag attached with a value.
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

/*
** wrong_attach_tag_value
**
** Attaches the tag with a value, which a tag has not.
*/
void wrong_attach_tag_value(void)
{
    uint32_t value = 1;

    bridge_attach(1, wrong_flag, bridge_lend(&value, sizeof(value)));
}
