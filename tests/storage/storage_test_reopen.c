/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "storage_test.h"

constexpr uint32_t REOPEN_MARK = 0xDEAD'BEEF;

static void run_only(storage_test_t *t, const char *phase)
{
    uint32_t i = 0;

    while (i < t->cfg.phase_count && strcmp(t->cfg.phases[i], phase))
        i++;
    schedule_run(t->w, i);
}

enum storage_test_end storage_test_publish_reopen(storage_test_t *t)
{
    bridge_world_t *w = t->w;
    uint32_t *typer;
    uint32_t table;

    if (storage_test_load_three(t, false))
        return (STORAGE_TEST_FAILED);
    storage_test_run(t, "spawn");
    storage_test_run(t, "attach_a");
    table = storage_test_find_table(w);
    typer = storage_test_cell_of(w, "game:typer@0.1.0", table);
    if (!table || !typer)
        return (STORAGE_TEST_FAILED);
    run_only(t, "destroy");
    block_hand_out(w);
    storage_apply(w->storage);
    if (!strcmp(t->more, "published"))
        block_barrier(w);
    *typer = REOPEN_MARK;
    run_only(t, "spawn");
    run_only(t, "attach_b");
    block_barrier(w);
    printf("table %s, cell of typer %s\n",
        storage_test_find_table(w) == table ? "taken again" : "another",
        *typer == REOPEN_MARK ? "kept" : "written");
    return (STORAGE_TEST_DONE);
}
