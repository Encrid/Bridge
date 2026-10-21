/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "host.h"

void host_print_record([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    const char *identity = r->identity ? r->identity : "-";
    const char *system = r->system ? r->system : "-";

    printf("record[%d] %s/%s: %.*s %u %u\n", r->level, identity, system,
        (int)r->len, r->text, r->arg[0], r->arg[1]);
}

void host_load(host_t *h, const bridge_artifact_t *a)
{
    const char *ended = bridge_load(h->w, a) ? "not loaded" : "loaded";

    printf("%s: %s\n", a->cfg.identity, ended);
}

void host_reload(host_t *h, const bridge_artifact_t *a)
{
    const char *ended = bridge_reload(h->w, a, 1) ? "not reloaded" : "reloaded";

    printf("%s: %s\n", a->cfg.identity, ended);
}

void host_unload(host_t *h, const char *identity)
{
    const char *ended = bridge_unload(h->w, identity) ? "not unloaded"
        : "unloaded";

    printf("%s: %s\n", identity, ended);
}

void host_run_frames(host_t *h, int count)
{
    int i = 0;

    while (i < count) {
        if (bridge_run_phase(h->w, "update"))
            printf("update refused\n");
        if (bridge_run_phase(h->w, "present"))
            printf("present refused\n");
        i++;
    }
}
