/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/storage/storage_test_single.c -- a singleton: one value, on no
** entity, and the three ways a module may not use one.
**
** storage.md, *A singleton*
*/

#include "storage_test.h"

/*
** Declared, and with no value. Then given one: heard, with the id 0.
*/
static const char *const SINGLE_GIVEN[] = {
    "look", "attach_a", "look", nullptr
};

/*
** value_a is another value: heard as an attach. `work` writes a field
** where it is, and nothing is heard. detach_b takes it, and attach_a gives
** it again: zero but for what the attach wrote.
*/
static const char *const SINGLE_KEPT[] = {
    "value_a", "look", "work", "look", "detach_b", "look", "attach_a", "look",
    nullptr
};

/*
** give_once
**
** Loads a build of the singleton module, looks, gives the singleton a
** value and looks again. Answers 1, having said so, if the load was
** refused.
*/
static int give_once(storage_test_t *t, const char *path)
{
    bridge_artifact_t a = storage_test_state(path, 8, 1 << 20, 4096);

    if (storage_test_try_load(t, &a))
        return (1);
    storage_test_run_all(t, SINGLE_GIVEN);
    return (0);
}

/*
** storage_test_give_single
**
** A singleton given, given again, written, taken and given; and unloaded
** with the policy delete, after which it has no value.
**
** lifetime.md, *What unload leaves behind*
*/
enum storage_test_end storage_test_give_single(storage_test_t *t)
{
    if (give_once(t, NSINGLE_PATH))
        return (STORAGE_TEST_LEFT);
    storage_test_run_all(t, SINGLE_KEPT);
    storage_test_drop(t, "st:G", BRIDGE_DELETE);
    storage_test_run(t, "look");
    return (STORAGE_TEST_DONE);
}

/*
** storage_test_walk_single
**
** A system whose term is a singleton is refused at load: it is on no
** entity a system walks.
*/
enum storage_test_end storage_test_walk_single(storage_test_t *t)
{
    (void)give_once(t, NSINGLEBAD1_PATH);
    return (STORAGE_TEST_LEFT);
}

/*
** storage_test_attach_single_to_entity
**
** A singleton attached to an entity is refused: its id is 0.
*/
enum storage_test_end storage_test_attach_single_to_entity(storage_test_t *t)
{
    (void)give_once(t, NSINGLEBAD2_PATH);
    return (STORAGE_TEST_LEFT);
}

/*
** storage_test_attach_to_no_entity
**
** A component that is no singleton attached to the id 0 is refused: it is
** no entity.
*/
enum storage_test_end storage_test_attach_to_no_entity(storage_test_t *t)
{
    (void)give_once(t, NSINGLEBAD3_PATH);
    return (STORAGE_TEST_LEFT);
}
