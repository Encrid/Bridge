/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "storage_test.h"

constexpr uint32_t PUBLISH_MARK = 0xDEAD'BEEF;

uint32_t *storage_test_cell_of(const bridge_world_t *w, const char *identity,
    uint32_t table)
{
    const module_t *m = w->modules;
    const bridge_header_t *h;

    while (m && strcmp(m->identity, identity))
        m = m->next;
    if (!m)
        return (nullptr);
    h = (const bridge_header_t *)(void *)m->priv;
    return ((uint32_t *)(void *)(m->priv + h->column_base)
        + (size_t)table * h->types);
}

uint32_t storage_test_find_table(const bridge_world_t *w)
{
    for (uint32_t i = 1; i < w->entities; i++)
        if (w->index[i].table)
            return (w->index[i].table);
    return (0);
}

int storage_test_load_three(storage_test_t *t, bool with_ok)
{
    bridge_artifact_t store = storage_test_state(STORE_PATH, 6, 1 << 20,
        1 << 20);
    bridge_artifact_t typer = storage_test_state(BUILT_DIR "/typer"
        BUILT_SUFFIX, BRIDGE_NONE, 1 << 20, BRIDGE_NONE);
    bridge_artifact_t ok = storage_test_state(OK_PATH, BRIDGE_NONE, 1 << 20,
        BRIDGE_NONE);

    typer.cfg.identity = "game:typer@0.1.0";
    ok.cfg.identity = "st:ok@0.1.0";
    if (storage_test_load(t, &store) || storage_test_load(t, &typer))
        return (1);
    return (with_ok && storage_test_load(t, &ok));
}

static const char *name_cell(const uint32_t *cell)
{
    return (*cell == PUBLISH_MARK ? "kept" : "written");
}

enum storage_test_end storage_test_publish_cells(storage_test_t *t)
{
    uint32_t *store;
    uint32_t *typer;
    uint32_t table;

    if (storage_test_load_three(t, true))
        return (STORAGE_TEST_FAILED);
    storage_test_run(t, "spawn");
    storage_test_run(t, "attach_a");
    table = storage_test_find_table(t->w);
    store = storage_test_cell_of(t->w, "st:store@0.1.0", table);
    typer = storage_test_cell_of(t->w, "game:typer@0.1.0", table);
    if (!table || !store || !typer)
        return (STORAGE_TEST_FAILED);
    *store = PUBLISH_MARK;
    *typer = PUBLISH_MARK;
    t->w->layout_moved = !strcmp(t->more, "full");
    storage_test_run(t, strcmp(t->more, "rows") ? "attach_b" : "destroy_half");
    printf("rows %u, cell of store %s, cell of typer %s\n",
        ((const uint32_t *)(void *)(t->w->shared + t->w->directory))[table],
        name_cell(store), name_cell(typer));
    return (STORAGE_TEST_DONE);
}
