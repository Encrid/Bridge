/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "bridge.h"

static void say([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    printf("%s: %.*s %u\n", r->identity ? r->identity : "bridge",
        (int)r->len, r->text, (unsigned)r->arg[0]);
}

static void state(bridge_artifact_t *a, const char *backend,
    const char *identity)
{
    *a = (bridge_artifact_t){ .backend = backend };
    a->cfg = (bridge_module_config_t){ .identity = identity,
        .abi = MODULE_ABI, .reloadable = BRIDGE_FALSE,
        .hot_reload = BRIDGE_FALSE, .pinned = BRIDGE_FALSE,
        .on_error = BRIDGE_SKIP, .max_system_failures = 3,
        .work_limit = BRIDGE_NO_LIMIT, .memory_limit = BRIDGE_NO_LIMIT,
        .init_timeout = BRIDGE_NO_LIMIT, .reload_timeout = BRIDGE_NO_LIMIT,
        .shutdown_timeout = BRIDGE_NO_LIMIT, .scratch = 64 << 10,
        .log_ring = 64 * 32, .spans = 16 << 10, .log_level = BRIDGE_INFO,
        .spawns = 4, .persistent = BRIDGE_NONE, .changes = 1 << 20,
        .loss_count = BRIDGE_NONE, .config = BRIDGE_NO_CONFIG };
}

static int load_module(bridge_world_t *w)
{
    bridge_artifact_t a;

    state(&a, "tests", "packed:module@0.1.0");
    a.path = MODULE;
    return (bridge_load(w, &a));
}

static const bridge_setting_t TOLD[4] = {
    { "capabilities", BRIDGE_INT, {}, { .i = 1 << 4 | 1 << 6 } },
    { "reservation", BRIDGE_INT, {}, { .i = 0 } },
    { "guard", BRIDGE_INT, {}, { .i = 0 } },
    { "granule", BRIDGE_INT, {}, { .i = 0 } }
};

int main(void)
{
    bridge_backend_t backends[1] = {{ "tests", BACKEND, TOLD, 4 }};
    const char *phases[1] = { "update" };
    bridge_config_t cfg = { .abi = BRIDGE_HOST_ABI, .threads = 1,
        .backends = backends, .backend_count = 1, .phases = phases,
        .phase_count = 1, .budget = { .entities = 64, .tables = 8, .types = 8,
        .systems = 8, .terms = 8, .modules = 4, .persistent = 4096,
        .columns = 1 << 16, .sparse_sets = BRIDGE_NONE,
        .direct_indexed = BRIDGE_NONE }};
    bridge_world_t *w = bridge_create(&cfg, say, nullptr);

    setvbuf(stdout, nullptr, _IONBF, 0);
    if (!w || load_module(w))
        return (printf("refused\n"), 1);
    for (int i = 0; i < 3; i++)
        (void)bridge_run_phase(w, "update");
    printf("destroyed: %d\n", bridge_destroy(w));
    return (0);
}
