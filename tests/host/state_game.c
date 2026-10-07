/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/host/state_game.c -- how this program states and loads a
** module.
*/

#include <string.h>

#include "cases.h"

/*
** test_state_wasm
**
** States a wasm module of tests/modules, with what it mints between two
** barriers.
*/
void test_state_wasm(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    bool few = !strcmp(file, "twostep") || !strcmp(file, "chain")
        || !strcmp(file, "events") || !strcmp(file, "nested")
        || !strcmp(file, "nested2");

    host_state_wasm(a, identity, file);
    if (few)
        a->cfg.spawns = 8;
    if (!strcmp(file, "mover"))
        a->cfg.spawns = 1024;
}

/*
** test_load_wasm
**
** Loads a wasm module as it is stated.
*/
void test_load_wasm(host_t *h, const char *identity, const char *file)
{
    bridge_artifact_t a;

    test_state_wasm(&a, identity, file);
    host_load(h, &a);
}

/*
** test_load_native
**
** Loads a native module that mints `spawns` ids between two barriers.
*/
void test_load_native(host_t *h, const char *identity, const char *path,
    int64_t spawns)
{
    bridge_artifact_t a;

    host_state_native(&a, identity, path);
    a.cfg.spawns = spawns;
    host_load(h, &a);
}

/*
** test_state_game_reader
**
** States the reader, with its grant.
*/
void test_state_game_reader(bridge_artifact_t *a)
{
    test_state_wasm(a, "view:reader@0.1.0", "reader");
    a->cfg.read = TEST_GAME;
    a->cfg.read_count = 2;
}

/*
** test_count_threads
**
** Answers how many distinct threads the napping modules said they ran on.
*/
uint32_t test_count_threads(const test_counts_t *c)
{
    uint32_t n = 0;
    uint32_t i = 0;
    uint32_t k;

    while (i < c->seen) {
        k = 0;
        while (k < i && c->threads[k] != c->threads[i])
            k++;
        n += k == i;
        i++;
    }
    return (n);
}
