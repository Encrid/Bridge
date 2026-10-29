/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge.h"

static void say([[maybe_unused]] void *ud,
    [[maybe_unused]] const bridge_record_t *r)
{
}

int main(void)
{
    const char *phases[1] = { "update" };
    bridge_config_t cfg = { .abi = BRIDGE_HOST_ABI, .threads = 1,
        .phases = phases, .phase_count = 1, .budget = { .entities = 64,
        .tables = 8, .types = 8, .systems = 8, .terms = 8, .modules = 4,
        .persistent = 4096, .columns = 1 << 16, .sparse_sets = BRIDGE_NONE,
        .direct_indexed = BRIDGE_NONE }, .profile = BRIDGE_FALSE,
        .validate = BRIDGE_FALSE, .observer_passes = 1, .stack = 1 << 20,
        .table_growth = 150, .table_hash_fill = 50, .table_retry = 50 };
    bridge_world_t *w = bridge_create(&cfg, say, nullptr);

    if (w)
        (void)bridge_destroy(w);
    return (0);
}
