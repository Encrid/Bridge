/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage_test.h"

static const char *const PHASES[] = {
    "spawn", "attach_a", "attach_b", "detach_b", "destroy", "attach_b_half",
    "detach_b_half", "detach_b_absent", "value_a", "flicker", "destroy_half",
    "respawn_half", "touch_dead", "attach_a_half", "recycled", "combine",
    "look", "never", "work", "check", "attach_both", "scribble", "find",
    "value_b", "look_b", "walk", "flag", "near", "far", "one"
};

static const bridge_setting_t KEEPS[4] = {
    { "capabilities", BRIDGE_INT, {}, { .i = 1 << 4 | 1 << 6 } },
    { "reservation", BRIDGE_INT, {}, { .i = 0 } },
    { "guard", BRIDGE_INT, {}, { .i = 0 } },
    { "granule", BRIDGE_INT, {}, { .i = 0 } }
};

static const bridge_setting_t KEEPS_ALL[4] = {
    { "capabilities", BRIDGE_INT, {}, { .i = 63 } },
    { "reservation", BRIDGE_INT, {}, { .i = INT64_C(1) << 32 } },
    { "guard", BRIDGE_INT, {}, { .i = 65'536 } },
    { "granule", BRIDGE_INT, {}, { .i = 65'536 } }
};

static const bridge_backend_t BACKENDS[2] = {
    { "guarded", TESTS_ALL_PATH, KEEPS_ALL, 4 },
    { "tests", TESTS_PATH, KEEPS, 4 }
};

static const storage_test_room_t PLAIN = { BACKENDS + 1, 1, 1, 32'768,
    64 << 20, BRIDGE_NONE, BRIDGE_NONE, false };
static const storage_test_room_t FOUR_THREADS = { BACKENDS + 1, 1, 4, 32'768,
    64 << 20, BRIDGE_NONE, BRIDGE_NONE, false };
static const storage_test_room_t BOTH_BACKENDS = { BACKENDS, 2, 1, 32'768,
    64 << 20, BRIDGE_NONE, BRIDGE_NONE, false };
static const storage_test_room_t ONE_TABLE = { BACKENDS + 1, 1, 1, 1,
    64 << 20, BRIDGE_NONE, BRIDGE_NONE, false };
static const storage_test_room_t COLUMNS_64 = { BACKENDS + 1, 1, 1, 32'768,
    64, BRIDGE_NONE, BRIDGE_NONE, false };
static const storage_test_room_t COLUMNS_160 = { BACKENDS + 1, 1, 1, 32'768,
    160, BRIDGE_NONE, BRIDGE_NONE, false };
static const storage_test_room_t COLUMNS_112 = { BACKENDS + 1, 1, 1, 32'768,
    112, BRIDGE_NONE, BRIDGE_NONE, false };
static const storage_test_room_t SPARSE = { BACKENDS + 1, 1, 1, 32'768,
    64 << 20, 1, BRIDGE_NONE, false };
static const storage_test_room_t SPARSE_112 = { BACKENDS + 1, 1, 1, 32'768,
    112, 1, BRIDGE_NONE, true };
static const storage_test_room_t DIRECT = { BACKENDS + 1, 1, 1, 32'768,
    64 << 20, BRIDGE_NONE, 1, false };
static const storage_test_room_t DIRECT_1024 = { BACKENDS + 1, 1, 1, 32'768,
    1024, BRIDGE_NONE, 1, false };
static const storage_test_room_t ONE_OF_EACH = { BACKENDS + 1, 1, 1, 32'768,
    64 << 20, 1, 1, false };

static const storage_test_case_t CASES[] = {
#ifdef COUNTED
    { "nomemory", storage_test_refuse_memory, &PLAIN },
    { "nomemoryrelay", storage_test_refuse_relay, &PLAIN },
#endif
    { "shape", storage_test_look_at_shape, &BOTH_BACKENDS },
    { "marks", storage_test_fill_marks, &PLAIN },
    { "matrix", storage_test_make_large_matrix, &PLAIN },
    { "matrixsmall", storage_test_make_small_matrix, &PLAIN },
    { "widerecord", storage_test_drain_wide_record, &PLAIN },
    { "pointaway", storage_test_point_query_away, &PLAIN },
    { "manytypes", storage_test_count_types, &PLAIN },
    { "manyall", storage_test_count_all, &PLAIN },
    { "manymove", storage_test_move_many, &PLAIN },
    { "twosingles", storage_test_give_two_singles, &PLAIN },
    { "fulllist", storage_test_fill_list, &ONE_TABLE },
    { "changes", storage_test_change, &PLAIN },
    { "tables", storage_test_make_tables, &PLAIN },
    { "notables", storage_test_use_up_tables, &ONE_TABLE },
    { "nocolumns", storage_test_use_up_columns, &COLUMNS_64 },
    { "scribble", storage_test_scribble, &PLAIN },
    { "strip", storage_test_strip, &PLAIN },
    { "stripfull", storage_test_strip_full, &COLUMNS_160 },
    { "layout", storage_test_relay, &PLAIN },
    { "move", storage_test_move, &PLAIN },
    { "tag", storage_test_tag, &ONE_OF_EACH },
    { "single", storage_test_give_single, &PLAIN },
    { "singlebad1", storage_test_walk_single, &PLAIN },
    { "singlebad2", storage_test_attach_single_to_entity, &PLAIN },
    { "singlebad3", storage_test_attach_to_no_entity, &PLAIN },
    { "direct", storage_test_index_directly, &DIRECT },
    { "directstrip", storage_test_drop_direct, &DIRECT },
    { "directsweep", storage_test_drop_direct, &DIRECT },
    { "directdelete", storage_test_drop_direct, &DIRECT },
    { "directnone", storage_test_drop_direct, &PLAIN },
    { "directfull", storage_test_drop_direct, &DIRECT_1024 },
    { "directbad", storage_test_drop_direct, &DIRECT },
    { "togglefull", storage_test_toggle_full, &COLUMNS_112 },
    { "toggle", storage_test_toggle, &PLAIN },
    { "sparse", storage_test_join_sparse, &SPARSE },
    { "sparsefull", storage_test_fill_sparse, &SPARSE_112 },
    { "sparsestrip", storage_test_drop_sparse, &SPARSE },
    { "sparsesweep", storage_test_drop_sparse, &SPARSE },
    { "sparsesweepa", storage_test_sweep_beside_sparse, &SPARSE },
    { "sparsedelete", storage_test_drop_sparse, &SPARSE },
    { "sparsenone", storage_test_drop_sparse, &PLAIN },
    { "sparsebad1", storage_test_drop_sparse, &SPARSE },
    { "sparsebad2", storage_test_drop_sparse, &SPARSE },
    { "stress", storage_test_stress, &FOUR_THREADS },
    { "stress1", storage_test_stress, &PLAIN },
    { "weigh", storage_test_weigh, &ONE_OF_EACH },
    { "baseline", storage_test_take_baseline, &PLAIN },
    { "measure", storage_test_measure, &PLAIN }
};

static const storage_test_case_t *find_case(const char *what)
{
    constexpr size_t count = sizeof(CASES) / sizeof(*CASES);
    size_t i = 0;

    while (i < count && strcmp(CASES[i].name, what))
        i++;
    return (i < count ? &CASES[i] : nullptr);
}

const storage_test_variant_t *storage_test_find_variant(
    const storage_test_variant_t *rows, size_t count, const char *what)
{
    size_t i = 0;

    while (i < count && strcmp(rows[i].what, what))
        i++;
    return (i < count ? &rows[i] : nullptr);
}

static void make_bridge(storage_test_t *t, const storage_test_room_t *room)
{
    constexpr uint32_t count = sizeof(PHASES) / sizeof(*PHASES);
    bridge_config_t cfg = { .abi = BRIDGE_HOST_ABI,
        .backends = room->backends, .backend_count = room->backend_count,
        .phases = PHASES, .phase_count = count, .threads = room->threads,
        .budget = { .entities = 400'000, .tables = room->tables, .types = 64,
            .systems = 64, .terms = 4, .modules = 4, .persistent = 4096,
            .columns = room->columns, .sparse_sets = room->sparse_sets,
            .direct_indexed = room->direct_indexed } };

    t->cfg = cfg;
    if (room->asks_columns && t->more)
        t->cfg.budget.columns = (uint64_t)atoi(t->more);
    t->b = bridge_create(&t->cfg, storage_test_hear, t);
    setvbuf(stdout, nullptr, _IONBF, 0);
}

int main(int argc, char **argv)
{
    storage_test_t t = { .what = argc > 1 ? argv[1] : "" };
    const storage_test_case_t *c = find_case(t.what);
    enum storage_test_end end;

    if (!c) {
        printf("FAIL: no case '%s'\n", t.what);
        return (1);
    }
    t.more = argc > 2 ? argv[2] : nullptr;
    make_bridge(&t, c->room);
    if (!t.b) {
        printf("FAIL: bridge_create\n");
        return (1);
    }
    end = c->run(&t);
    if (end == STORAGE_TEST_DONE)
        (void)bridge_destroy(t.b);
    if (end != STORAGE_TEST_FAILED)
        printf("alive\n");
    return (end == STORAGE_TEST_FAILED);
}
