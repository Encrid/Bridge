/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage3/stage3_share.c -- one column and two modules, and what a
** second declaration of a component may and may not be.
*/

#include "stage3.h"

/*
** stage3_share
**
** One column, two modules: the mover writes it and the reader, of another
** namespace, reads it through its grant.
*/
void stage3_share(host_t *h)
{
    if (host_open(h))
        return;
    stage3_load_mover(h);
    stage3_load_reader(h);
    host_run_frames(h, 3);
    host_close(h);
}

/*
** stage3_mirror
**
** A module that follows references to its own entities.
*/
void stage3_mirror(host_t *h)
{
    if (host_open(h))
        return;
    stage3_load(h, "game:refs@0.1.0", "refs");
    host_run_frames(h, 3);
    host_close(h);
}

/*
** stage3_declare_other_layout
**
** A second module declares the mover's component with another component
** layout, and is refused by its fingerprint.
*/
void stage3_declare_other_layout(host_t *h)
{
    if (host_open(h))
        return;
    stage3_load_mover(h);
    stage3_load(h, "game:fp@0.1.0", "fp");
    host_close(h);
}

/*
** stage3_declare_other_storage
**
** A storage layout nobody stated, one that is none of the five, and
** another than the mover's: each is refused.
*/
void stage3_declare_other_storage(host_t *h)
{
    if (host_open(h))
        return;
    stage3_load_mover(h);
    stage3_load(h, "game:kept@0.1.0", "kept-unset");
    stage3_load(h, "game:kept@0.1.0", "kept-none");
    stage3_load(h, "game:kept@0.1.0", "kept-sparse");
    host_close(h);
}

/*
** stage3_declare_ungranted
**
** The reader, with no grant: it may not declare what is another
** namespace's.
*/
void stage3_declare_ungranted(host_t *h)
{
    if (host_open(h))
        return;
    stage3_load(h, "view:reader@0.1.0", "reader");
    host_close(h);
}
