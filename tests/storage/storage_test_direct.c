/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

static const char *const DIRECT_GIVEN[] = {
    "spawn", "attach_b", "look_b", "near", nullptr
};

static const char *const DIRECT_KEPT[] = {
    "flicker", "look_b", "value_b", "near", "detach_b_half", "look_b", "destroy_half", "look_b",
    "near", "respawn_half", "look_b", "near", "detach_b", "look_b", "destroy",
    "look_b", nullptr
};

static const storage_test_variant_t DIRECT_DROPPED[] = {
    { "directstrip", DIRECT_PATH, BRIDGE_STRIP },
    { "directsweep", DIRECT_PATH, BRIDGE_SWEEP },
    { "directdelete", DIRECT_PATH, BRIDGE_DELETE },
    { "directnone", DIRECT_PATH, BRIDGE_DELETE },
    { "directfull", DIRECT_PATH, BRIDGE_DELETE },
    { "directbad", DIRECTBAD_PATH, BRIDGE_DELETE }
};

static int give_direct(storage_test_t *t, const char *path)
{
    bridge_artifact_t a = storage_test_state(path, 8, 1 << 20, 4096);

    if (storage_test_try_load(t, &a))
        return (1);
    storage_test_run_all(t, DIRECT_GIVEN);
    return (0);
}

enum storage_test_end storage_test_index_directly(storage_test_t *t)
{
    if (give_direct(t, DIRECT_PATH))
        return (STORAGE_TEST_LEFT);
    storage_test_run_all(t, DIRECT_KEPT);
    return (STORAGE_TEST_DONE);
}

static void say_alive(const storage_test_t *t)
{
    uint32_t e = 1;
    bool alive;

    while (e <= 6) {
        alive = storage_is_alive(t->b->storage, (uint64_t)1 << 32 | e);
        printf("entity %u is %s\n", e, alive ? "alive" : "gone");
        e++;
    }
}

enum storage_test_end storage_test_drop_direct(storage_test_t *t)
{
    constexpr size_t count = sizeof(DIRECT_DROPPED) / sizeof(*DIRECT_DROPPED);
    const storage_test_variant_t *v = storage_test_find_variant(
        DIRECT_DROPPED, count, t->what);

    if (give_direct(t, v->path))
        return (STORAGE_TEST_LEFT);
    storage_test_drop(t, "st:A", BRIDGE_STRIP);
    storage_test_drop(t, "st:D", v->policy);
    say_alive(t);
    storage_test_run(t, "near");
    return (STORAGE_TEST_DONE);
}
