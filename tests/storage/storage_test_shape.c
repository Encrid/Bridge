/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

static const char *say_if(bool held)
{
    return (held ? "yes" : "NO");
}

enum storage_test_end storage_test_look_at_shape(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);
    const bridge_world_t *w = t->w;

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    printf("the shared block reserves more than it uses: %s\n",
        say_if(w->shared_reserve > w->shared_len));
    printf("a plain module's private block reserves what it uses: %s\n",
        say_if(w->modules->priv_reserve == w->modules->priv_len));
    printf("the shared block is whole guarded pages: %s\n",
        say_if(w->shared_len % 65'536 == 0));
    printf("a plain module's private block is not rounded to one: %s\n",
        say_if(w->modules->priv_len % 65'536 != 0));
    return (STORAGE_TEST_DONE);
}
