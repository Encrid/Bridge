/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/packaged/host.c -- a host as one outside the tree is: built from the
** library's package and nothing else of Bridge.
**
** host_api.md, *What a host is handed*
*/

#include <stdio.h>
#include <stdlib.h>

#include "bridge.h"

/*
** say
**
** The sink: a record, on a line.
*/
static void say([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    printf("%s: %.*s %u\n", r->identity ? r->identity : "bridge",
        (int)r->len, r->text, (unsigned)r->arg[0]);
}

/*
** state
**
** States every key of a module, for the backend named.
*/
static void state(bridge_artifact_t *a, const char *backend,
    const char *identity)
{
    *a = (bridge_artifact_t){ .backend = backend };
    a->cfg = (bridge_module_config_t){ .identity = identity,
        .abi = MODULE_ABI, .trusted = BRIDGE_TRUE, .reloadable = BRIDGE_FALSE,
        .hot_reload = BRIDGE_FALSE, .pinned = BRIDGE_FALSE,
        .on_error = BRIDGE_SKIP, .max_system_failures = 3,
        .work_limit = BRIDGE_NO_LIMIT, .memory_limit = BRIDGE_NO_LIMIT,
        .init_timeout = BRIDGE_NO_LIMIT, .reload_timeout = BRIDGE_NO_LIMIT,
        .shutdown_timeout = BRIDGE_NO_LIMIT, .scratch = 64 << 10,
        .log_ring = 64 * 24, .spans = 16 << 10, .log_level = BRIDGE_INFO,
        .spawns = 4, .persistent = BRIDGE_NONE, .changes = 1 << 20,
        .loss_count = BRIDGE_NONE, .config = BRIDGE_NO_CONFIG };
}

/*
** load_wasm
**
** Loads the wasm module, whose bytes a host reads itself.
*/
static int load_wasm(bridge_t *b)
{
    bridge_artifact_t a;
    FILE *f = fopen(WASM_MODULE, "rb");
    static char bytes[1 << 20];

    if (!f)
        return (1);
    state(&a, "wasm", "packed:wasm@0.1.0");
    a.bytes = bytes;
    a.len = fread(bytes, 1, sizeof(bytes), f);
    fclose(f);
    a.cfg.trusted = BRIDGE_FALSE;
    a.cfg.work_limit = 10'000'000;
    a.cfg.memory_limit = 16 << 20;
    return (bridge_load(b, &a));
}

/*
** load_native
**
** Loads the native module, by the path of its library.
*/
static int load_native(bridge_t *b)
{
    bridge_artifact_t a;

    state(&a, "native", "packed:native@0.1.0");
    a.path = NATIVE_MODULE;
    return (bridge_load(b, &a));
}

/*
** main
**
** A deck with both backends, a module on each, three passes, and the deck
** destroyed.
*/
int main(void)
{
    bridge_backend_t backends[2] = {{ "wasm", WASM_BACKEND },
        { "native", NATIVE_BACKEND }};
    const char *phases[1] = { "update" };
    bridge_config_t cfg = { .abi = BRIDGE_HOST_ABI, .threads = 1,
        .backends = backends, .backend_count = 2, .phases = phases,
        .phase_count = 1, .budget = { .entities = 64, .tables = 8, .types = 8,
        .systems = 8, .terms = 8, .modules = 4, .persistent = 4096,
        .columns = 1 << 16, .sparse_sets = BRIDGE_NONE,
        .direct_indexed = BRIDGE_NONE }};
    bridge_t *b = bridge_create(&cfg, say, nullptr);

    setvbuf(stdout, nullptr, _IONBF, 0);
    if (!b || load_native(b) || load_wasm(b))
        return (printf("refused\n"), 1);
    for (int i = 0; i < 3; i++)
        (void)bridge_run_phase(b, "update");
    printf("destroyed: %d\n", bridge_destroy(b));
    return (0);
}
