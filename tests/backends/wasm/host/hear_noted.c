/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "cases.h"

static bool begins(const bridge_record_t *r, const char *text, size_t len)
{
    return (r->len >= len && !memcmp(r->text, text, len));
}

void test_hear_noted(void *ud, const bridge_record_t *r)
{
    test_noted_t *more = ud;

    if (r->len == 7 && begins(r, "path at", 7) && more->path_count < 4) {
        more->path_at[more->path_count] = r->arg[0];
        more->path_count++;
    }
    if (r->len > 40 && begins(r, "the entity index is out of step", 31))
        more->out_of_step++;
    if (more->quiet && r->level > BRIDGE_WARN)
        return;
    host_print_record(nullptr, r);
}

void test_load_wasm_minting(host_t *h, const char *identity, const char *file)
{
    bridge_artifact_t a;

    test_state_wasm_minting(&a, identity, file);
    host_load(h, &a);
}

void test_load_spawnless(host_t *h, const char *identity, const char *file)
{
    bridge_artifact_t a;

    test_state_wasm_minting(&a, identity, file);
    a.cfg.spawns = BRIDGE_NONE;
    host_load(h, &a);
}

void test_say_modules(host_t *h)
{
    bridge_module_info_t info[8];
    int n = bridge_list_modules(h->w, info, 8);
    int i = 0;

    printf("modules:");
    while (i < n && i < 8) {
        printf(" %s=%s", info[i].identity,
            info[i].state == BRIDGE_STOPPED ? "STOPPED" : "RUNNING");
        i++;
    }
    printf(" (%d)\n", n);
}
