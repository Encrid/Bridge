/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "adopt.h"

static const enum backend_type ADOPT_PARAMS[2] = { BACKEND_I32, BACKEND_I64 };
static const enum backend_type ADOPT_RESULTS[1] = { BACKEND_I32 };

static int ping([[maybe_unused]] void *ud, const uint64_t *args,
    uint64_t *results)
{
    results[0] = args[0] + args[1];
    return (0);
}

static backend_instance_t *make_host(adopt_t *t, backend_module_t **host,
    enum backend_capability demands)
{
    backend_sig_t sig = { .params = ADOPT_PARAMS, .param_count = 2,
        .results = ADOPT_RESULTS, .result_count = 1 };
    backend_entry_t entry = { "bridge", "ping", ping, nullptr, &sig };
    backend_source_t source = { .demands = demands };
    backend_instance_t *i = nullptr;

    source.bytes = adopt_read_file(MODULE_DIR "/host.wasm", &source.len);
    *host = t->api.module_compile(t->be, &source, &t->err);
    if (*host)
        i = t->api.instance_create(*host, nullptr, 0, &t->err);
    if (i && t->api.define_func(i, &entry, &t->err))
        return (nullptr);
    return (i);
}

int adopt_check_host_function(adopt_t *t)
{
    backend_module_t *host;
    backend_instance_t *i = make_host(t, &host, 0);
    int32_t status = -1;

    if (!i || adopt_run(t, i, "init", &status) || status != 16)
        return (adopt_fail("a host function: %d: %s", status, t->err.text));
    printf("a host function: called with an i32 and an i64, answered %d\n",
        status);
    t->api.instance_destroy(i);
    t->api.module_destroy(host);
    return (0);
}

static int call_timed(adopt_t *t, backend_instance_t *i, backend_func_t *f,
    int32_t *status)
{
    int k = 0;

    while (k < 20'000) {
        t->api.instance_set_timeout(i, 1'000'000'000ull);
        if (t->api.call(f, status, &t->err) || *status != 16)
            return (k);
        k++;
    }
    return (k);
}

int adopt_check_timed_calls(adopt_t *t)
{
    backend_module_t *timed;
    backend_instance_t *i = make_host(t, &timed, BACKEND_TIMEOUT);
    backend_func_t *f = nullptr;
    int32_t status = -1;
    int k;

    if (i)
        f = t->api.instance_get_func(i, "init", &t->err);
    if (!f)
        return (adopt_fail("timed calls: %s", t->err.text));
    k = call_timed(t, i, f, &status);
    if (k < 20'000)
        return (adopt_fail("timed call %d: %d: %s", k, status, t->err.text));
    printf("20000 timed calls back to back: none interrupted\n");
    t->api.instance_destroy(i);
    t->api.module_destroy(timed);
    return (0);
}
