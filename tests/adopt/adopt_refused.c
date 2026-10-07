/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/adopt/adopt_refused.c -- what the wasm backend refuses of whoever
** calls it: a block that is no whole number of pages, and a function
** defined too late or twice.
*/

#include <stdio.h>

#include "adopt.h"

static const enum backend_type ADOPT_STATUS[1] = { BACKEND_I32 };

static int answer([[maybe_unused]] void *ud,
    [[maybe_unused]] const uint64_t *args, uint64_t *results)
{
    results[0] = 0;
    return (0);
}

/*
** adopt_say_refused
**
** Says why a call was refused. Answers 1, having said so, if it was
** accepted.
*/
int adopt_say_refused(const char *what, bool refused,
    const backend_err_t *err)
{
    if (!refused)
        return (adopt_fail("%s was accepted", what));
    printf("%s is refused: %s\n", what, err->text);
    return (0);
}

static int refuse_block(adopt_t *t)
{
    backend_err_t err = {};
    const backend_memory_t *m = t->api.memory_adopt(t->be, t->block, 1000,
        &err);

    return (adopt_say_refused("a block of 1000 bytes", !m, &err));
}

/*
** refuse_function
**
** Defines a function for an instance twice. Answers whether the second was
** refused; the first is, too, for an instance that has run.
*/
static bool refuse_function(adopt_t *t, backend_instance_t *i,
    backend_err_t *err)
{
    backend_sig_t sig = { .results = ADOPT_STATUS, .result_count = 1 };
    backend_entry_t entry = { "bridge", "twice", answer, nullptr, &sig };

    if (t->api.define_func(i, &entry, err))
        return (true);
    return (t->api.define_func(i, &entry, err) != 0);
}

/*
** adopt_check_functions
**
** A function defined for an instance that has run, and one defined twice
** for an instance that has not, are each refused; and so is that block.
*/
int adopt_check_functions(adopt_t *t, backend_instance_t *one)
{
    backend_err_t err = {};
    backend_instance_t *fresh = adopt_make_instance(t);
    int failed = refuse_block(t);

    failed += adopt_say_refused("a function defined after the first call",
        refuse_function(t, one, &err), &err);
    failed += adopt_say_refused("a function defined twice",
        refuse_function(t, fresh, &err), &err);
    t->api.instance_destroy(fresh);
    return (failed != 0);
}
