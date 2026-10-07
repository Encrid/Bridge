/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage2/stage2_unset.c -- keys nobody stated: each is refused, and
** none is taken for a default.
**
** config.md, *No unaccountable values*
*/

#include "stage2.h"

/*
** stage2_state_no_host_abi
**
** A host that stated no ABI is refused.
*/
void stage2_state_no_host_abi(host_t *h)
{
    h->cfg.abi = 0;
    if (host_open(h))
        return;
    stage2_load(h, h->what);
    host_close(h);
}

/*
** stage2_leave_unset
**
** A limit left at zero is unset, and refused.
*/
void stage2_leave_unset(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage2_state(&a, identity, "ok");
    a.cfg.memory_limit = 0;
    host_load(h, &a);
    host_close(h);
}

/*
** stage2_leave_level_unset
**
** A log level left at zero is refused: zero is a severity, and is unset all
** the same.
*/
void stage2_leave_level_unset(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage2_state(&a, identity, "ok");
    a.cfg.log_level = 0;
    host_load(h, &a);
    host_close(h);
}

/*
** stage2_leave_flag_unset
**
** A yes or no nobody stated is not a no, and a value that is neither is
** refused too.
*/
void stage2_leave_flag_unset(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage2_state(&a, identity, "ok");
    a.cfg.pinned = BRIDGE_UNSET;
    host_load(h, &a);
    stage2_state(&a, identity, "ok");
    a.cfg.trusted = (enum bridge_flag)3;
    host_load(h, &a);
    host_close(h);
}

/*
** stage2_state_negative
**
** A negative number that is no "none" at all is refused.
*/
void stage2_state_negative(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage2_state(&a, identity, "ok");
    a.cfg.memory_limit = -7;
    host_load(h, &a);
    host_close(h);
}
