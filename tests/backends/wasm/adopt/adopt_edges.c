/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "adopt.h"

static int edge_keyless(adopt_t *t)
{
    backend_setting_t settings[1] = {{ .kind = BACKEND_BOOL }};
    backend_err_t err = {};
    backend_t *b = t->api.backend_create(settings, 1, &err);

    if (b)
        t->api.backend_destroy(b);
    return (adopt_say_refused("a setting with no key", !b, &err));
}

static int edge_wide(adopt_t *t)
{
    backend_err_t err = {};
    backend_instance_t *i = adopt_make_instance(t);
    backend_sig_t params = { .param_count = 17 };
    backend_sig_t results = { .result_count = 5 };
    backend_entry_t wide = { "bridge", "wide", nullptr, nullptr, &params };
    int failed;

    failed = adopt_say_refused("a function of seventeen parameters",
        t->api.define_func(i, &wide, &err) != 0, &err);
    wide.sig = &results;
    failed += adopt_say_refused("a function of five results",
        t->api.define_func(i, &wide, &err) != 0, &err);
    t->api.instance_destroy(i);
    return (failed != 0);
}

static int edge_source(adopt_t *t)
{
    backend_err_t err = {};
    unsigned char byte = 0;
    backend_source_t none = {};
    backend_source_t empty = { .bytes = &byte };
    int failed;

    failed = adopt_say_refused("a module from no bytes",
        !t->api.module_compile(t->be, &none, &err), &err);
    failed += adopt_say_refused("a module from empty bytes",
        !t->api.module_compile(t->be, &empty, &err), &err);
    return (failed != 0);
}

static int edge_adopt_bad(adopt_t *t)
{
    backend_err_t err = {};
    int failed;

    failed = adopt_say_refused("a block at no address",
        !t->api.memory_adopt(t->be, nullptr, 2 * ADOPT_PAGE, &err), &err);
    failed += adopt_say_refused("a block of no length",
        !t->api.memory_adopt(t->be, t->block, 0, &err), &err);
    return (failed != 0);
}

static int edge_imports(adopt_t *t)
{
    backend_import_t out[1] = {};

    if (!t->api.module_list_imports(t->module, out, 0))
        return (adopt_fail("the adopt module counts no import"));
    printf("a room of none lists none of its imports\n");
    return (0);
}

static int edge_unmetered(adopt_t *t, backend_instance_t *one)
{
    t->api.instance_set_work_limit(one, 1000);
    printf("a work limit on an unmetered module is ignored\n");
    return (0);
}

static backend_instance_t *edge_open_module(adopt_t *t, const char *path,
    backend_module_t **module)
{
    backend_err_t err = {};
    backend_memory_import_t imports[1] = {{ "bridge", "shared", t->memory }};
    backend_source_t source = {};

    source.bytes = adopt_read_file(path, &source.len);
    *module = t->api.module_compile(t->be, &source, &err);
    if (!*module)
        return (nullptr);
    return (t->api.instance_create(*module, imports, 1, &err));
}

static int edge_own_grow(adopt_t *t)
{
    backend_module_t *module = nullptr;
    backend_instance_t *i = edge_open_module(t, MODULE_DIR "/grow.wasm",
        &module);
    int32_t status = 7;
    int failed = 1;

    if (i && adopt_run(t, i, "init", &status) == 0 && status == 0)
        failed = 0;
    printf("a module that grows its own memory answered %d\n", status);
    if (i)
        t->api.instance_destroy(i);
    if (module)
        t->api.module_destroy(module);
    return (failed);
}

static int edge_exports(adopt_t *t)
{
    backend_module_t *module = nullptr;
    backend_instance_t *i = edge_open_module(t, MODULE_DIR "/edges.wasm",
        &module);
    int failed;

    if (!i)
        return (adopt_fail("edges did not make an instance"));
    failed = adopt_say_refused("an export that answers nothing",
        !t->api.instance_get_func(i, "voidinit", &t->err), &t->err);
    failed += adopt_say_refused("an export that is not a function",
        !t->api.instance_get_func(i, "glob", &t->err), &t->err);
    t->api.instance_destroy(i);
    t->api.module_destroy(module);
    return (failed != 0);
}

static int edge_trap(adopt_t *t)
{
    backend_module_t *module = nullptr;
    backend_instance_t *i = edge_open_module(t, MODULE_DIR "/trapstart.wasm",
        &module);
    int failed;

    if (!i)
        return (adopt_fail("trapstart did not make an instance"));
    failed = adopt_say_refused("a module whose start traps",
        !t->api.instance_get_func(i, "init", &t->err), &t->err);
    t->api.instance_destroy(i);
    t->api.module_destroy(module);
    return (failed);
}

static int edge_trap_silent(adopt_t *t)
{
    backend_module_t *module = nullptr;
    backend_instance_t *i = edge_open_module(t, MODULE_DIR "/stray.wasm",
        &module);

    if (!i)
        return (adopt_fail("stray did not make an instance"));
    if (t->api.instance_get_func(i, "init", nullptr))
        return (adopt_fail("a module that cannot instantiate found a function"));
    printf("a module that cannot instantiate, with no room for a reason, is "
        "refused in silence\n");
    t->api.instance_destroy(i);
    t->api.module_destroy(module);
    return (0);
}

static int edge_three(adopt_t *t)
{
    backend_err_t err = {};
    backend_memory_import_t three[3] = {{ "bridge", "shared", t->memory },
        { "bridge", "shared", t->memory }, { "bridge", "spare", t->memory }};
    backend_instance_t *i = t->api.instance_create(t->module, three, 3, &err);

    if (i)
        t->api.instance_destroy(i);
    return (adopt_say_refused("three memories, the second a repeated name",
        !i, &err));
}

static int edge_no_err(adopt_t *t, backend_instance_t *one)
{
    if (t->api.instance_get_func(one, "missing", nullptr))
        return (adopt_fail("a missing export was found"));
    printf("a missing export with no room for a reason is refused in silence\n");
    return (0);
}

int adopt_check_edges(adopt_t *t, backend_instance_t *one)
{
    return (edge_keyless(t) || edge_wide(t) || edge_source(t)
        || edge_adopt_bad(t) || edge_imports(t) || edge_unmetered(t, one)
        || edge_own_grow(t) || edge_exports(t) || edge_trap(t)
        || edge_trap_silent(t) || edge_three(t) || edge_no_err(t, one));
}
