/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/hear_noted.c -- the sink of this program, how it loads a
** module, and the pull.
*/

#include <stdio.h>
#include <string.h>

#include "cases.h"

static bool begins(const bridge_record_t *r, const char *text, size_t len)
{
    return (r->len >= len && !memcmp(r->text, text, len));
}

/*
** test_hear_noted
**
** The sink of this program: keeps what the cases count, and prints a
** record unless the program is quiet and it is below a warning.
*/
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

/*
** test_load_guarded_minting
**
** Loads a guarded module as it is stated.
*/
void test_load_guarded_minting(host_t *h, const char *identity,
    const char *file)
{
    bridge_artifact_t a;

    test_state_guarded_minting(&a, identity, file);
    host_load(h, &a);
}

/*
** test_load_spawnless
**
** Loads a guarded module, stating that it spawns nothing.
*/
void test_load_spawnless(host_t *h, const char *identity, const char *file)
{
    bridge_artifact_t a;

    test_state_guarded_minting(&a, identity, file);
    a.cfg.spawns = BRIDGE_NONE;
    host_load(h, &a);
}

/*
** test_say_modules
**
** The pull: says what is loaded, and whether it still runs.
*/
void test_say_modules(host_t *h)
{
    bridge_module_info_t info[8];
    int n = bridge_list_modules(h->b, info, 8);
    int i = 0;

    printf("modules:");
    while (i < n && i < 8) {
        printf(" %s=%s", info[i].identity,
            info[i].state == BRIDGE_STOPPED ? "STOPPED" : "RUNNING");
        i++;
    }
    printf(" (%d)\n", n);
}
