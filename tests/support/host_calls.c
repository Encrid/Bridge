/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/support/host_calls.c -- the calls a host makes, each said as it
** ended, and the sink that prints a record.
*/

#include <stdio.h>

#include "host.h"

/*
** host_print_record
**
** A sink: prints a record with whose it is and its two numbers.
*/
void host_print_record([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    const char *identity = r->identity ? r->identity : "-";
    const char *system = r->system ? r->system : "-";

    printf("record[%d] %s/%s: %.*s %u %u\n", r->level, identity, system,
        (int)r->len, r->text, r->arg[0], r->arg[1]);
}

/*
** host_load
**
** Loads a module and says how it ended.
*/
void host_load(host_t *h, const bridge_artifact_t *a)
{
    const char *ended = bridge_load(h->b, a) ? "not loaded" : "loaded";

    printf("%s: %s\n", a->cfg.identity, ended);
}

/*
** host_reload
**
** Reloads a module and says how it ended.
*/
void host_reload(host_t *h, const bridge_artifact_t *a)
{
    const char *ended = bridge_reload(h->b, a) ? "not reloaded" : "reloaded";

    printf("%s: %s\n", a->cfg.identity, ended);
}

/*
** host_unload
**
** Unloads a module and says how it ended.
*/
void host_unload(host_t *h, const char *identity)
{
    const char *ended = bridge_unload(h->b, identity) ? "not unloaded"
        : "unloaded";

    printf("%s: %s\n", identity, ended);
}

/*
** host_run_frames
**
** Runs "update" and "present" `count` times, and says each that was
** refused.
*/
void host_run_frames(host_t *h, int count)
{
    int i = 0;

    while (i < count) {
        if (bridge_run_phase(h->b, "update"))
            printf("update refused\n");
        if (bridge_run_phase(h->b, "present"))
            printf("present refused\n");
        i++;
    }
}
