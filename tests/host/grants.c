/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/grants.c -- one column and two modules, and what a
** second declaration of a component may and may not be.
*/

#include "cases.h"

/*
** test_share
**
** One column, two modules: the mover writes it and the reader, of another
** namespace, reads it through its grant.
*/
void test_share(host_t *h)
{
    if (host_open(h))
        return;
    test_load_mover(h);
    test_load_reader(h);
    host_run_frames(h, 3);
    host_close(h);
}

/*
** test_mirror
**
** A module that follows references to its own entities.
*/
void test_mirror(host_t *h)
{
    if (host_open(h))
        return;
    test_load_module(h, "game:refs@0.1.0", "refs");
    host_run_frames(h, 3);
    host_close(h);
}

/*
** test_declare_other_layout
**
** A second module declares the mover's component with another component
** layout, and is refused by its fingerprint.
*/
void test_declare_other_layout(host_t *h)
{
    if (host_open(h))
        return;
    test_load_mover(h);
    test_load_module(h, "game:fp@0.1.0", "fp");
    host_close(h);
}

/*
** test_declare_other_storage
**
** A storage layout nobody stated, one that is none of the five, and
** another than the mover's: each is refused.
*/
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

/*
** test_declare_ungranted
**
** The reader, with no grant: it may not declare what is another
** namespace's.
*/
void test_declare_ungranted(host_t *h)
{
    if (host_open(h))
        return;
    test_load_module(h, "view:reader@0.1.0", "reader");
    host_close(h);
}
