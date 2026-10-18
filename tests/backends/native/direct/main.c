/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "backend.h"

static int refuse_entry(const backend_api_t *api, backend_module_t *m)
{
    backend_err_t err = {};
    backend_entry_t entry = { "bridge", "teleport", nullptr, nullptr,
        nullptr };
    backend_instance_t *i = api->instance_create(m, nullptr, 0, &err);
    int refused = api->define_func(i, &entry, &err);

    api->instance_destroy(i);
    if (!refused)
        return (printf("FAIL: an entry the SDK has not was kept\n"), 1);
    printf("an entry the SDK has not is refused: %s\n", err.text);
    return (0);
}

static void poke_edges(const backend_api_t *api, backend_t *be,
    backend_module_t *m)
{
    backend_err_t err = {};
    backend_setting_t keyless = {};
    backend_entry_t wrong = { "other", "spawn", nullptr, nullptr, nullptr };
    backend_source_t empty = { .path = "" };
    backend_source_t missing = { .path = "no-such-library" };
    backend_import_t out[2] = {};
    backend_instance_t *i = api->instance_create(m, nullptr, 0, &err);

    (void)api->backend_create(&keyless, 1, &err);
    (void)api->module_compile(be, &empty, &err);
    (void)api->module_compile(be, &missing, &err);
    (void)api->module_list_imports(m, out, 1);
    (void)api->define_func(i, &wrong, &err);
    api->instance_set_work_limit(i, 1);
    api->instance_set_timeout(i, 1);
    api->instance_set_memory_limit(i, 1);
    api->instance_destroy(i);
}

static void poke_missing(const backend_api_t *api, backend_t *be)
{
    backend_err_t err = {};
    backend_source_t source = { .path = NCOVER_PATH };
    backend_module_t *m = api->module_compile(be, &source, &err);
    backend_instance_t *i = api->instance_create(m, nullptr, 0, &err);
    backend_func_t *init = api->instance_get_func(i, "init", &err);
    int32_t status = 0;

    (void)api->call(init, &status, &err);
    api->instance_destroy(i);
    api->module_destroy(m);
}

int main(void)
{
    backend_api_t api;
    backend_err_t err = {};
    backend_source_t source = { .path = NMOVER_PATH };
    backend_t *be;
    backend_module_t *m;

    setvbuf(stdout, nullptr, _IONBF, 0);
    if (!backend_open(NATIVE_PATH, &api, &err))
        return (printf("FAIL: %s\n", err.text), 1);
    be = api.backend_create(nullptr, 0, &err);
    m = api.module_compile(be, &source, &err);
    if (!m || refuse_entry(&api, m))
        return (printf("FAIL: %s\n", err.text), 1);
    poke_edges(&api, be, m);
    poke_missing(&api, be);
    api.module_destroy(m);
    api.backend_destroy(be);
    return (0);
}
