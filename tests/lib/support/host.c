/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "host.h"

static const bridge_setting_t HOST_KEEPS[4] = {
    { "capabilities", BRIDGE_INT, {}, { .i = HOST_TESTS_KEEPS } },
    { "reservation", BRIDGE_INT, {}, { .i = 0 } },
    { "guard", BRIDGE_INT, {}, { .i = 0 } },
    { "granule", BRIDGE_INT, {}, { .i = 0 } }
};

static const bridge_setting_t HOST_KEEPS_ALL[4] = {
    { "capabilities", BRIDGE_INT, {}, { .i = HOST_TESTS_KEEPS_ALL } },
    { "reservation", BRIDGE_INT, {}, { .i = HOST_TESTS_RESERVES } },
    { "guard", BRIDGE_INT, {}, { .i = HOST_TESTS_GRANULE } },
    { "granule", BRIDGE_INT, {}, { .i = HOST_TESTS_GRANULE } }
};

void host_init(host_t *h, int argc, char **argv)
{
    bridge_budget_t budget = { .entities = 4096, .tables = 64, .types = 64,
        .systems = 128, .terms = 8, .modules = 8, .persistent = 4096,
        .columns = 1 << 20, .sparse_sets = BRIDGE_NONE,
        .direct_indexed = BRIDGE_NONE };

    setvbuf(stdout, nullptr, _IONBF, 0);
    *h = (host_t){ .what = argc > 1 ? argv[1] : "" };
    h->backends[0] = (bridge_backend_t){ "guarded", TESTS_ALL_PATH,
        HOST_KEEPS_ALL, 4 };
    h->backends[1] = (bridge_backend_t){ "plain", TESTS_PATH, HOST_KEEPS, 4 };
    h->phases[0] = "update";
    h->phases[1] = "present";
    h->cfg = (bridge_config_t){ .abi = BRIDGE_HOST_ABI, .threads = 1 };
    h->cfg.backends = h->backends;
    h->cfg.backend_count = 2;
    h->cfg.phases = h->phases;
    h->cfg.phase_count = 2;
    h->cfg.budget = budget;
}

int host_run(host_t *h, const host_case_t *cases, size_t count)
{
    size_t i = 0;

    while (i < count && strcmp(cases[i].name, h->what))
        i++;
    if (i == count) {
        printf("no case '%s'\n", h->what);
        return (2);
    }
    h->arg = cases[i].arg;
    cases[i].set(h);
    cases[i].run(h);
    return (0);
}

int host_open(host_t *h)
{
    h->w = bridge_create(&h->cfg, h->sink, h->ud);
    if (h->w)
        return (0);
    printf("bridge_create refused\nalive\n");
    return (1);
}

void host_close(host_t *h)
{
    (void)bridge_destroy(h->w);
    printf("alive\n");
}

void host_open_only(host_t *h)
{
    if (!host_open(h))
        host_close(h);
}
