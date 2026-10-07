/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage2/stage2_refused.c -- what is refused for whose it is: an ABI
** that is another's, a backend there is not, a library that cannot be
** opened, no sink.
*/

#include <stdio.h>

#include "stage2.h"

/*
** stage2_create_no_sink
**
** A bridge asked for with no sink is refused.
*/
void stage2_create_no_sink(host_t *h)
{
    bridge_t *b = bridge_create(&h->cfg, nullptr, nullptr);

    printf("%s\n", b ? "created" : "refused: no sink");
    printf("alive\n");
}

/*
** stage2_state_other_abi
**
** A module built for another ABI is refused.
*/
void stage2_state_other_abi(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage2_state(&a, identity, "ok");
    a.cfg.abi = BRIDGE_ABI + 1;
    host_load(h, &a);
    host_close(h);
}

/*
** stage2_name_no_backend
**
** A module that names a backend the deployment has not is refused.
*/
void stage2_name_no_backend(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage2_state(&a, identity, "ok");
    a.backend = "native";
    host_load(h, &a);
    host_close(h);
}

/*
** stage2_open_no_library
**
** A backend whose library cannot be opened refuses the bridge.
*/
void stage2_open_no_library(host_t *h)
{
    h->backends[0].library = "no-such-backend-library";
    if (host_open(h))
        return;
    stage2_load(h, h->what);
    host_close(h);
}

/*
** stage2_state_other_host_abi
**
** A host built against another bridge.h is refused: nothing else of its
** configuration is read.
*/
void stage2_state_other_host_abi(host_t *h)
{
    h->cfg.abi = BRIDGE_HOST_ABI + 1;
    if (host_open(h))
        return;
    stage2_load(h, h->what);
    host_close(h);
}
