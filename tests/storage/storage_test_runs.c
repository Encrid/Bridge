/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "abi_entry.h"
#include "storage_test.h"

constexpr uint32_t RUNS_MOST = 8;

typedef struct {
    char letter;
    uint32_t turn;
    uint64_t id;
} queued_run_t;

static declared_type_t *find_position(const module_t *more)
{
    uint32_t i = 0;

    while (i < more->declared_count
        && strcmp(more->declared[i]->name, "st:Position"))
        i++;
    return (i < more->declared_count ? more->declared[i] : nullptr);
}

static int queue_run(bridge_world_t *w, declared_type_t *c, queued_run_t *run)
{
    module_t *m = storage_test_find_timed(w, (uint32_t)(run->letter - 'a') + 1);
    change_t ch = { 0, c, c->size, CHANGE_ATTACH };

    if (!m || m->id_next == m->id_count || !abi_marks(m, 1))
        return (1);
    run->id = m->ids[m->id_next];
    m->id_next++;
    ch.id = run->id;
    m->turn = TURN_SYSTEM | run->turn;
    if (queue_change(m, &ch, c->zero))
        return (1);
    block_note_work(w, m);
    return (0);
}

static bool queue_all(bridge_world_t *w, declared_type_t *c, const char *plan,
    queued_run_t *runs, uint32_t *count)
{
    uint32_t turn = 0;

    for (const char *at = plan; *at && *count < RUNS_MOST; at++) {
        if (*at == '=') {
            turn--;
            continue;
        }
        runs[*count] = (queued_run_t){ *at, turn, 0 };
        if (queue_run(w, c, &runs[*count]))
            return (false);
        (*count)++;
        turn++;
    }
    return (true);
}

static void say_rows(const bridge_world_t *w, const queued_run_t *runs,
    uint32_t count)
{
    printf("rows:");
    for (uint32_t row = 0; row < count; row++)
        for (uint32_t i = 0; i < count; i++)
            if (w->index[(uint32_t)runs[i].id].row == row)
                printf(" %c%u", runs[i].letter, runs[i].turn);
    printf("\n");
}

enum storage_test_end storage_test_runs(storage_test_t *t)
{
    queued_run_t runs[RUNS_MOST] = {};
    uint32_t count = 0;
    module_t *more;
    declared_type_t *c;

    if (storage_test_make_timed(t, 3, 1))
        return (STORAGE_TEST_FAILED);
    more = storage_test_find_timed(t->w, 0);
    c = more ? find_position(more) : nullptr;
    if (!c || !queue_all(t->w, c, t->more, runs, &count)) {
        printf("FAIL: the runs were not queued\n");
        return (STORAGE_TEST_FAILED);
    }
    block_note_work(t->w, more);
    block_barrier(t->w);
    say_rows(t->w, runs, count);
    return (STORAGE_TEST_DONE);
}
