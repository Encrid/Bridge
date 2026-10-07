/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/storage/storage_test_sparse.c -- a component kept in a sparse set:
** through every change, with no room for all its rows, and what unload
** leaves of it.
**
** storage.md, *A sparse set*
*/

#include <stdio.h>

#include "storage_test.h"

/*
** Three of six entities join the set, and are heard.
*/
static const char *const SPARSE_JOINED[] = {
    "spawn", "attach_b", "look", "look_b", nullptr
};

/*
** value_b is a value for one that is in the set: heard as an attach. With
** detach_b_half one leaves, and the set's last row takes its place.
** attach_b_half makes an entity that is in the set and in no table.
** destroy_half destroys one that is in the set, and one that is not. With
** detach_b the rest leave, and the lone one stays.
*/
static const char *const SPARSE_KEPT[] = {
    "value_b", "look", "detach_b_half", "look", "look_b", "attach_b_half",
    "look", "look_b", "destroy_half", "look", "look_b", "detach_b", "look",
    "destroy", "look", "look_b", nullptr
};

/*
** After the drop it is a set that can be joined again.
*/
static const char *const SPARSE_DROPPED[] = {
    "look", "look_b", "attach_b", "look", nullptr
};

/*
** The cases that load a build, fill the set and drop it. The one that
** states there are no sparse sets and the two whose system selects by S
** are each refused at load.
*/
static const storage_test_variant_t SPARSE_DROPS[] = {
    { "sparsestrip", NSPARSE_PATH, BRIDGE_STRIP },
    { "sparsesweep", NSPARSE_PATH, BRIDGE_SWEEP },
    { "sparsedelete", NSPARSE_PATH, BRIDGE_DELETE },
    { "sparsenone", NSPARSE_PATH, BRIDGE_DELETE },
    { "sparsebad1", NSPARSEBAD1_PATH, BRIDGE_DELETE },
    { "sparsebad2", NSPARSEBAD2_PATH, BRIDGE_DELETE }
};

/*
** join_sparse
**
** Loads a build of the sparse set module, which is stopped at this many
** failures, and has three of six entities join the set. Answers 1, having
** said so, if the load was refused.
*/
static int join_sparse(storage_test_t *t, const char *path, uint32_t failures)
{
    bridge_artifact_t a = storage_test_state(path, 8, 1 << 20, 4096);

    a.cfg.max_system_failures = failures;
    if (storage_test_try_load(t, &a))
        return (1);
    storage_test_run_all(t, SPARSE_JOINED);
    return (0);
}

/*
** storage_test_join_sparse
**
** A sparse set joined, given a value, left, and on entities that are
** destroyed.
*/
enum storage_test_end storage_test_join_sparse(storage_test_t *t)
{
    if (join_sparse(t, NSPARSE_PATH, 1))
        return (STORAGE_TEST_LEFT);
    storage_test_run_all(t, SPARSE_KEPT);
    return (STORAGE_TEST_DONE);
}

/*
** storage_test_fill_sparse
**
** A set with room for two of the three: one barrier leaves an entity out,
** and the module, which has a budget of four failures, is looked at after
** it.
**
** budgets.md, *What running out does*
*/
enum storage_test_end storage_test_fill_sparse(storage_test_t *t)
{
    if (join_sparse(t, NSPARSE_PATH, 4))
        return (STORAGE_TEST_LEFT);
    return (STORAGE_TEST_DONE);
}

/*
** storage_test_sweep_beside_sparse
**
** A sweep of a component in a table: an entity that is in a sparse set
** carries something still, and stays, in no table.
*/
enum storage_test_end storage_test_sweep_beside_sparse(storage_test_t *t)
{
    if (join_sparse(t, NSPARSE_PATH, 1))
        return (STORAGE_TEST_LEFT);
    storage_test_drop(t, "st:A", BRIDGE_SWEEP);
    storage_test_run(t, "look");
    storage_test_run(t, "look_b");
    return (STORAGE_TEST_DONE);
}

/*
** storage_test_drop_sparse
**
** What unload leaves behind of a component in a sparse set, by the policy
** the case is named for. The seventh entity made is the one that carried S
** and nothing else.
**
** lifetime.md, *What unload leaves behind*
*/
enum storage_test_end storage_test_drop_sparse(storage_test_t *t)
{
    constexpr size_t count = sizeof(SPARSE_DROPS) / sizeof(*SPARSE_DROPS);
    const storage_test_variant_t *v = storage_test_find_variant(SPARSE_DROPS,
        count, t->what);

    if (join_sparse(t, v->path, 1))
        return (STORAGE_TEST_LEFT);
    storage_test_run(t, "attach_b_half");
    storage_test_drop(t, "st:S", v->policy);
    printf("the entity that was in the set alone is %s\n",
        storage_is_alive(t->b->storage, (uint64_t)1 << 32 | 7) ? "alive" :
        "gone");
    storage_test_run_all(t, SPARSE_DROPPED);
    return (STORAGE_TEST_DONE);
}
