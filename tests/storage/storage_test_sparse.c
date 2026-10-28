/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "storage_test.h"

static const char *const SPARSE_JOINED[] = {
    "spawn", "attach_b", "look", "look_b", nullptr
};

static const char *const SPARSE_KEPT[] = {
    "value_b", "look", "detach_b_half", "look", "look_b", "attach_b_half",
    "look", "look_b", "destroy_half", "look", "look_b", "detach_b", "look",
    "destroy", "look", "look_b", nullptr
};

static const char *const SPARSE_DROPPED[] = {
    "look", "look_b", "attach_b", "look", nullptr
};

static const storage_test_variant_t SPARSE_DROPS[] = {
    { "sparsestrip", SPARSE_PATH, BRIDGE_STRIP },
    { "sparsesweep", SPARSE_PATH, BRIDGE_SWEEP },
    { "sparsedelete", SPARSE_PATH, BRIDGE_DELETE },
    { "sparsenone", SPARSE_PATH, BRIDGE_DELETE },
    { "sparsebad1", SPARSEBAD1_PATH, BRIDGE_DELETE },
    { "sparsebad2", SPARSEBAD2_PATH, BRIDGE_DELETE },
    { "sparsehusk", SPARSEHUSK_PATH, BRIDGE_DELETE }
};

static int join_sparse(storage_test_t *t, const char *path, uint32_t failures)
{
    bridge_artifact_t a = storage_test_state(path, 8, 1 << 20, 4096);

    a.cfg.max_system_failures = failures;
    if (storage_test_try_load(t, &a))
        return (1);
    storage_test_run_all(t, SPARSE_JOINED);
    return (0);
}

enum storage_test_end storage_test_join_sparse(storage_test_t *t)
{
    if (join_sparse(t, SPARSE_PATH, 1))
        return (STORAGE_TEST_DONE);
    storage_test_run_all(t, SPARSE_KEPT);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_fill_sparse(storage_test_t *t)
{
    if (join_sparse(t, SPARSE_PATH, 4))
        return (STORAGE_TEST_DONE);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_sweep_beside_sparse(storage_test_t *t)
{
    if (join_sparse(t, SPARSE_PATH, 1))
        return (STORAGE_TEST_DONE);
    storage_test_drop(t, "st:A", BRIDGE_SWEEP);
    storage_test_run(t, "look");
    storage_test_run(t, "look_b");
    return (STORAGE_TEST_DONE);
}

static enum storage_test_end husk_sparse(storage_test_t *t,
    const storage_test_variant_t *v)
{
    storage_test_drop(t, "st:A", v->policy);
    storage_test_say_alive(t, 7);
    storage_test_drop(t, "st:S", BRIDGE_STRIP);
    storage_test_say_alive(t, 7);
    storage_test_run(t, "look");
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_drop_sparse(storage_test_t *t)
{
    constexpr size_t count = sizeof(SPARSE_DROPS) / sizeof(*SPARSE_DROPS);
    const storage_test_variant_t *v = storage_test_find_variant(SPARSE_DROPS,
        count, t->what);

    if (join_sparse(t, v->path, 1))
        return (STORAGE_TEST_DONE);
    storage_test_run(t, "attach_b_half");
    if (!strcmp(t->what, "sparsehusk"))
        return (husk_sparse(t, v));
    storage_test_drop(t, "st:S", v->policy);
    printf("the entity that was in the set alone is %s\n",
        storage_is_alive(t->w->storage, (uint64_t)1 << 32 | 7) ? "alive" :
        "gone");
    storage_test_run_all(t, SPARSE_DROPPED);
    return (STORAGE_TEST_DONE);
}
