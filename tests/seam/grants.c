/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cases.h"

void test_share(host_t *h)
{
    const char *file = strchr(h->arg, ' ') + 1;
    const char *reads = strchr(file, ' ') + 1;
    char identity[64];
    char name[32];
    bridge_artifact_t a;

    snprintf(identity, sizeof(identity), "%.*s", (int)(file - 1 - h->arg),
        h->arg);
    snprintf(name, sizeof(name), "%.*s", (int)(reads - 1 - file), file);
    if (host_open(h))
        return;
    test_load_mover(h);
    test_state_reader_as(&a, identity, name);
    a.cfg.read_count = (uint32_t)strtol(reads, nullptr, 10);
    host_load(h, &a);
    host_run_frames(h, 3);
    host_close(h);
}

void test_mirror(host_t *h)
{
    if (host_open(h))
        return;
    test_load_module(h, "game:refs@0.1.0", "refs");
    host_run_frames(h, (int)strtol(h->arg, nullptr, 10));
    host_close(h);
}

void test_declare_other_layout(host_t *h)
{
    if (host_open(h))
        return;
    test_load_mover(h);
    test_load_module(h, "game:fp@0.1.0", h->arg);
    host_close(h);
}

void test_declare_other_storage(host_t *h)
{
    if (host_open(h))
        return;
    test_load_mover(h);
    test_load_module(h, "game:kept@0.1.0", h->arg);
    test_load_module(h, "game:kept@0.1.0", "kept-none");
    test_load_module(h, "game:kept@0.1.0", "kept-sparse");
    host_close(h);
}

void test_declare_other_on_destroy(host_t *h)
{
    if (host_open(h))
        return;
    test_load_mover(h);
    test_load_module(h, "game:kept@0.1.0", h->arg);
    host_close(h);
}

void test_declare_ungranted(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_reader(&a);
    a.cfg.read_count = (uint32_t)strtol(h->arg, nullptr, 10);
    host_load(h, &a);
    host_close(h);
}
