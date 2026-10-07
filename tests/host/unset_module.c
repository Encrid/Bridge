/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/unset_module.c -- keys nobody stated: each is refused, and
** none is taken for a default.
**
** config.md, *No unaccountable values*
*/

#include "cases.h"

/*
** test_state_no_host_abi
**
** A host that stated no ABI is refused.
*/
void test_state_no_host_abi(host_t *h)
{
    h->cfg.abi = 0;
    if (host_open(h))
        return;
    test_load_small(h, h->arg);
    host_close(h);
}

/*
** test_leave_unset
**
** A limit left at zero is unset, and refused.
*/
void test_leave_unset(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.memory_limit = 0;
    host_load(h, &a);
    host_close(h);
}

/*
** test_leave_level_unset
**
** A log level left at zero is refused: zero is a severity, and is unset all
** the same.
*/
void test_leave_level_unset(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.log_level = 0;
    host_load(h, &a);
    host_close(h);
}

/*
** test_leave_flag_unset
**
** A yes or no nobody stated is not a no, and a value that is neither is
** refused too.
*/
void test_leave_flag_unset(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.pinned = BRIDGE_UNSET;
    host_load(h, &a);
    test_state_small(&a, identity, "ok");
    a.cfg.trusted = (enum bridge_flag)3;
    host_load(h, &a);
    host_close(h);
}

/*
** test_state_negative
**
** A negative number that is no "none" at all is refused.
*/
void test_state_negative(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.memory_limit = -7;
    host_load(h, &a);
    host_close(h);
}
