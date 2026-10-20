/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage.h"
#include "storage_test.h"

static uint32_t heard_attaches;

static void hear_attach(void *ctx, uint64_t id, enum bridge_event event)
{
    (void)ctx;
    (void)id;
    heard_attaches += (event & BRIDGE_ON_ATTACH) != 0;
}

enum storage_test_end storage_test_give_two_singles(storage_test_t *t)
{
    storage_t *r = t->w->storage;
    storage_type_t *a = storage_declare(r, 4, 4, BRIDGE_SINGLETON);
    storage_type_t *b = storage_declare(r, 4, 4, BRIDGE_SINGLETON);
    storage_term_t term = { a, BRIDGE_AND };
    storage_listener_t hear = { BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH
        | BRIDGE_ON_LEAVE, hear_attach, nullptr };
    uint32_t value = 7;

    (void)storage_add_observer(r, &term, 1, &hear);
    storage_give_single(r, a, &value, 4);
    heard_attaches = 0;
    storage_give_single(r, b, &value, 4);
    printf("A observer heard ON_ATTACH when B got its value: %u\n",
        heard_attaches);
    return (STORAGE_TEST_DONE);
}
