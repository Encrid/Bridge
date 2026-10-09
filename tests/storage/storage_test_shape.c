/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test_shape.c -- how much address space each block
** reserves, and how long each is, beside a backend that maps what it is
** handed.
**
** backend.md, *Memory is adopted, not created*
*/

#include <stdio.h>

#include "storage_test.h"

static const char *say_if(bool held)
{
    return (held ? "yes" : "NO");
}

/*
** storage_test_look_at_shape
**
** A plain module's private block reserves what it uses, though the guarded
** backend is there and wants four gigabytes. And each block is as long as
** who maps it needs: the shared block whole guarded pages, since the guarded
** backend maps it, and a plain module's private block not.
*/
enum storage_test_end storage_test_look_at_shape(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);
    const bridge_t *b = t->b;

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    printf("the shared block reserves more than it uses: %s\n",
        say_if(b->shared_reserve > b->shared_len));
    printf("a plain module's private block reserves what it uses: %s\n",
        say_if(b->modules->priv_reserve == b->modules->priv_len));
    printf("the shared block is whole guarded pages: %s\n",
        say_if(b->shared_len % 65'536 == 0));
    printf("a plain module's private block is not rounded to one: %s\n",
        say_if(b->modules->priv_len % 65'536 != 0));
    return (STORAGE_TEST_DONE);
}
