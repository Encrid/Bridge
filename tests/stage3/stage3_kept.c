/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage3/stage3_kept.c -- the storage layouts that are not a table,
** as a wasm module reaches them, and a load that fails.
**
** storage.md, *Storage layouts*
*/

#include "stage3.h"

/*
** stage3_reach_direct
**
** A direct-indexed component, reached from a table's rows.
*/
void stage3_reach_direct(host_t *h)
{
    bridge_artifact_t a;

    h->cfg.budget.direct_indexed = 1;
    if (host_open(h))
        return;
    stage3_state(&a, "game:aim@0.1.0", "aim");
    a.cfg.spawns = 16;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

/*
** stage3_count_toggled
**
** A toggle: counted before any is turned off, and after.
*/
void stage3_count_toggled(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage3_state(&a, "game:shield@0.1.0", "shield");
    a.cfg.spawns = 16;
    host_load(h, &a);
    (void)bridge_run_phase(h->b, "present");
    host_run_frames(h, 1);
    host_close(h);
}

/*
** stage3_walk_sparse
**
** A sparse set, walked and reached.
*/
void stage3_walk_sparse(host_t *h)
{
    bridge_artifact_t a;

    h->cfg.budget.sparse_sets = 1;
    if (host_open(h))
        return;
    stage3_state(&a, "game:hit@0.1.0", "hit");
    a.cfg.spawns = 16;
    host_load(h, &a);
    host_run_frames(h, 2);
    host_close(h);
}

/*
** stage3_state_bad_term
**
** A module whose system states a term a query may not ask for.
*/
void stage3_state_bad_term(host_t *h)
{
    if (host_open(h))
        return;
    stage3_load(h, "game:badterm@0.1.0", "badterm");
    host_close(h);
}

/*
** stage3_undo_load
**
** A load that fails after declaring and spawning leaves nothing: the same
** component name with another layout then loads, which a ghost of the first
** would refuse.
*/
void stage3_undo_load(host_t *h)
{
    if (host_open(h))
        return;
    stage3_load(h, "game:badterm@0.1.0", "badterm");
    stage3_load(h, "game:fp@0.1.0", "fp");
    host_close(h);
}
