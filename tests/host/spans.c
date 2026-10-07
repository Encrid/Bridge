/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/host/spans.c -- a module's spans, outgrown at a barrier: they held
** what its queries matched when it was loaded, and do not hold what they
** match once its entities are there.
*/

#include "cases.h"

/*
** test_outgrow_spans
**
** The mover, with the 544 bytes of spans a load asks for under this
** budget, which hold where its columns are and no span: its query matches
** no table at load, and one at the first barrier.
*/
void test_outgrow_spans(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "game:mover@0.1.0", "mover");
    a.cfg.spans = 544;
    host_load(h, &a);
    host_run_frames(h, 2);
    host_close(h);
}
