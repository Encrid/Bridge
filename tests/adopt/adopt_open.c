/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/adopt/adopt_open.c -- the backend opened, a block adopted, and the
** module compiled.
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "adopt.h"

/*
** adopt_fail
**
** Says that a check failed. Answers 1.
*/
int adopt_fail(const char *format, ...)
{
    va_list args;

    printf("FAIL: ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    return (1);
}

/*
** adopt_read_file
**
** Answers the bytes of a file and how many.
*/
void *adopt_read_file(const char *path, size_t *len)
{
    FILE *f = fopen(path, "rb");
    void *data = nullptr;

    if (f) {
        fseek(f, 0, SEEK_END);
        *len = (size_t)ftell(f);
        fseek(f, 0, SEEK_SET);
        data = malloc(*len);
    }
    if (!data || fread(data, 1, *len, f) != *len)
        exit(adopt_fail("cannot read %s", path));
    fclose(f);
    return (data);
}

/*
** adopt_make_instance
**
** Answers a store of the module with the block as its memory.
*/
backend_instance_t *adopt_make_instance(adopt_t *t)
{
    backend_err_t err = {};
    backend_memory_import_t imports[1] = {{ "bridge", "shared", t->memory }};
    backend_instance_t *i = t->api.instance_create(t->module, imports, 1,
        &err);

    if (!i)
        exit(adopt_fail("instance_create: %s", err.text));
    return (i);
}

/*
** adopt_run
**
** Calls a function of an instance by name.
*/
int adopt_run(adopt_t *t, backend_instance_t *i, const char *name,
    int32_t *status)
{
    backend_func_t *f = t->api.instance_get_func(i, name, &t->err);

    if (!f)
        return (-1);
    return (t->api.call(f, status, &t->err));
}

/*
** adopt_open
**
** Opens the wasm backend, reserves a block of the shape it states, which is
** Bridge's, has the backend adopt two pages of it, and compiles the
** module.
*/
int adopt_open(adopt_t *t)
{
    backend_err_t err = {};
    backend_source_t source = {};
    uint64_t shape[3];

    if (!backend_open(BACKEND_PATH, &t->api, &err))
        return (adopt_fail("%s", err.text));
    t->be = t->api.backend_create(nullptr, 0, &err);
    t->api.memory_get_shape(t->be, &shape[0], &shape[1], &shape[2]);
    printf("shape: reservation %llu MiB, guard %llu MiB\n",
        (unsigned long long)(shape[0] >> 20),
        (unsigned long long)(shape[1] >> 20));
    t->reserved = (size_t)(shape[0] + shape[1]);
    t->block = os_reserve(t->reserved, 2 * ADOPT_PAGE);
    if (!t->block)
        return (adopt_fail("cannot reserve the block"));
    t->memory = t->api.memory_adopt(t->be, t->block, 2 * ADOPT_PAGE, &err);
    source.bytes = adopt_read_file(MODULE_DIR "/adopt.wasm", &source.len);
    t->module = t->api.module_compile(t->be, &source, &err);
    return (t->module ? 0 : adopt_fail("module_compile: %s", err.text));
}
