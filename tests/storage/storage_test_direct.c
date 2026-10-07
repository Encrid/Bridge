/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/storage/storage_test_direct.c -- a direct-indexed component: a
** value reached by an id that is held, through every change, and what
** unload leaves of it.
**
** storage.md, *Direct-indexed*
*/

#include <stdio.h>

#include "storage_test.h"

/*
** Three of six entities are given D, and are heard.
*/
static const char *const DIRECT_GIVEN[] = {
    "spawn", "attach_b", "look_b", "near", nullptr
};

/*
** value_b is a value for one that has it: heard as an attach. With
** detach_b_half one loses it. destroy_half destroys one that has it, and
** one that has not, and respawn_half makes two more in the places of those
** two: with no D.
*/
static const char *const DIRECT_KEPT[] = {
    "value_b", "near", "detach_b_half", "look_b", "destroy_half", "look_b",
    "near", "respawn_half", "look_b", "near", "detach_b", "look_b", "destroy",
    "look_b", nullptr
};

/*
** The cases that load a build, give D and drop it. The one that states
** there are no direct-indexed components, the one with no room for the
** cells and the one whose system selects by D are each refused at load.
*/
static const storage_test_variant_t DIRECT_DROPPED[] = {
    { "directstrip", NDIRECT_PATH, BRIDGE_STRIP },
    { "directsweep", NDIRECT_PATH, BRIDGE_SWEEP },
    { "directdelete", NDIRECT_PATH, BRIDGE_DELETE },
    { "directnone", NDIRECT_PATH, BRIDGE_DELETE },
    { "directfull", NDIRECT_PATH, BRIDGE_DELETE },
    { "directbad", NDIRECTBAD_PATH, BRIDGE_DELETE }
};

/*
** give_direct
**
** Loads a build of the direct-indexed module and gives D to three of six
** entities. Answers 1, having said so, if the load was refused.
*/
static int give_direct(storage_test_t *t, const char *path)
{
    bridge_artifact_t a = storage_test_state(path, 8, 1 << 20, 4096);

    if (storage_test_try_load(t, &a))
        return (1);
    storage_test_run_all(t, DIRECT_GIVEN);
    return (0);
}

/*
** storage_test_index_directly
**
** A direct-indexed component given, given again, taken, and on entities
** that are destroyed and made again.
*/
enum storage_test_end storage_test_index_directly(storage_test_t *t)
{
    if (give_direct(t, NDIRECT_PATH))
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

/*
** storage_test_drop_direct
**
** What unload leaves behind of a direct-indexed component, by the policy
** the case is named for. A is stripped first, so that some entities carry
** D and nothing else.
**
** lifetime.md, *What unload leaves behind*
*/
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
