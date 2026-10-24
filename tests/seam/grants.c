/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_share(host_t *h)
{
    if (host_open(h))
        return;
    test_load_mover(h);
    test_load_reader(h);
    host_run_frames(h, 3);
    host_close(h);
}

void test_mirror(host_t *h)
{
    if (host_open(h))
        return;
    test_load_module(h, "game:refs@0.1.0", "refs");
    host_run_frames(h, 3);
    host_close(h);
}

void test_declare_other_layout(host_t *h)
{
    if (host_open(h))
        return;
    test_load_mover(h);
    test_load_module(h, "game:fp@0.1.0", "fp");
    host_close(h);
}

void test_declare_other_storage(host_t *h)
{
    if (host_open(h))
        return;
    test_load_mover(h);
    test_load_module(h, "game:kept@0.1.0", "kept-unset");
    test_load_module(h, "game:kept@0.1.0", "kept-none");
    test_load_module(h, "game:kept@0.1.0", "kept-sparse");
    host_close(h);
}

void test_declare_ungranted(host_t *h)
{
    if (host_open(h))
        return;
    test_load_module(h, "view:reader@0.1.0", "reader");
    host_close(h);
}
