/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/refused_native.c -- what a backend cannot keep or cannot
** find, refused at the load.
**
** capabilities.md, *Nothing is downgraded*
*/

#include "cases.h"

/*
** test_state_untrusted
**
** A module that is not trusted, on a backend that runs only modules that
** are: refused before any library is looked for.
*/
void test_state_untrusted(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_native(&a, "game:mover@0.1.0", "no-such-library");
    a.cfg.trusted = BRIDGE_FALSE;
    host_load(h, &a);
    host_close(h);
}

/*
** test_state_limit
**
** A work limit or a memory limit, as the case's row says, on a backend that
** keeps neither.
*/
void test_state_limit(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_native(&a, "game:mover@0.1.0", "no-such-library");
    if (*h->arg == 'w')
        a.cfg.work_limit = 1000;
    if (*h->arg == 'm')
        a.cfg.memory_limit = 1 << 20;
    host_load(h, &a);
    host_close(h);
}

/*
** test_state_no_path
**
** A native module, and nowhere to find it.
*/
void test_state_no_path(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_native(&a, "game:mover@0.1.0", nullptr);
    host_load(h, &a);
    host_close(h);
}

/*
** test_state_no_bytes
**
** A wasm module, and nothing to compile.
*/
void test_state_no_bytes(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_reader_limited(&a);
    a.bytes = nullptr;
    a.len = 0;
    host_load(h, &a);
    host_close(h);
}

/*
** test_name_no_library
**
** A native module whose library cannot be opened.
*/
void test_name_no_library(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_native(&a, "game:mover@0.1.0", "no-such-library");
    host_load(h, &a);
    host_close(h);
}
