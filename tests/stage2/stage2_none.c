/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage2/stage2_none.c -- a key's "none": it is a value, and each
** key has its own.
**
** config.md, *No unaccountable values*
*/

#include "stage2.h"

/*
** stage2_state_no_limit
**
** "None" is a value, and a module that states it for every limit loads.
*/
void stage2_state_no_limit(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage2_state(&a, identity, "ok");
    a.cfg.work_limit = BRIDGE_NO_LIMIT;
    a.cfg.memory_limit = BRIDGE_NO_LIMIT;
    a.cfg.init_timeout = BRIDGE_NO_LIMIT;
    a.cfg.reload_timeout = BRIDGE_NO_LIMIT;
    a.cfg.shutdown_timeout = BRIDGE_NO_LIMIT;
    host_load(h, &a);
    host_close(h);
}

/*
** stage2_state_other_none
**
** The "none" of another key is not this one's: an amount stated as no
** limit, and a limit stated as no amount, are each refused.
*/
void stage2_state_other_none(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage2_state(&a, identity, "ok");
    a.cfg.spawns = BRIDGE_NO_LIMIT;
    host_load(h, &a);
    stage2_state(&a, identity, "ok");
    a.cfg.work_limit = BRIDGE_NONE;
    host_load(h, &a);
    host_close(h);
}
