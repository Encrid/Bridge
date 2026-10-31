/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_STORAGE_TEST_H
    #define TEST_STORAGE_TEST_H

    #include "inside.h"

constexpr uint32_t STORAGE_TEST_ROWS = 204'800;
constexpr uint32_t STORAGE_TEST_WEIGH_ROUNDS = 8;
constexpr uint32_t STORAGE_TEST_OPS = 10;
constexpr uint32_t STORAGE_TEST_MEASURE_ROUNDS = 6;
constexpr uint32_t BOTH_MODULES = 6;
constexpr uint32_t BOTH_SYSTEMS = 3;
constexpr uint32_t BOTH_TYPES = 5;
constexpr uint32_t BOTH_ROOM = 24;

enum storage_test_end : int {
    STORAGE_TEST_DONE,
    STORAGE_TEST_FAILED
};

typedef struct {
    bridge_config_t cfg;
    bridge_world_t *w;
    const char *what;
    const char *more;
    uint32_t said[2];
    uint64_t called;
} storage_test_t;

typedef struct {
    const bridge_backend_t *backends;
    uint32_t backend_count;
    uint32_t threads;
    uint32_t tables;
    uint64_t columns;
    int64_t sparse_sets;
    int64_t direct_indexed;
    bool asks_columns;
    bool makes_world;
} storage_test_room_t;

typedef struct {
    const char *name;
    enum storage_test_end (*run)(storage_test_t *t);
    const storage_test_room_t *room;
} storage_test_case_t;

typedef struct {
    const char *what;
    const char *path;
    enum bridge_on_unload policy;
} storage_test_variant_t;

typedef struct {
    const char *from;
    const char *to;
    const char *what;
    const char *const *losses;
    int64_t loss_count;
} storage_test_relaid_t;

typedef struct {
    uint32_t from;
    uint32_t to;
    const char *prepare;
    const char *const *losses;
    int64_t loss_count;
} storage_test_moved_t;

typedef struct {
    const char *path;
    uint32_t bytes;
    uint32_t count;
} storage_test_carried_t;

typedef struct {
    const char *path;
    uint32_t share;
} storage_test_spread_t;

typedef struct {
    const char *phase;
    const char *what;
} storage_test_op_t;

typedef struct {
    uint64_t first[STORAGE_TEST_OPS];
    uint64_t best[STORAGE_TEST_OPS];
    uint64_t calls[STORAGE_TEST_OPS];
} storage_test_times_t;

typedef struct {
    uint64_t on;
    uint64_t off;
    uint64_t on_call;
    uint64_t off_call;
} storage_test_change_t;

void storage_test_hear(void *ud, const bridge_record_t *r);
void storage_test_hear_nothing(void *ud, const bridge_record_t *r);

void storage_test_hear_memory(void *ud, const bridge_record_t *r);

void storage_test_hear_counts(void *ud, const bridge_record_t *r);


bridge_artifact_t storage_test_state(const char *path, int64_t spawns,
    uint32_t spans, int64_t changes);

int storage_test_load(storage_test_t *t, const bridge_artifact_t *a);
int storage_test_try_load(storage_test_t *t, const bridge_artifact_t *a);

void storage_test_load_reloadable(bridge_world_t *w, bridge_artifact_t *a);

int storage_test_reopen(storage_test_t *t, bridge_sink_fn_t sink,
    const bridge_artifact_t *a);

const storage_test_variant_t *storage_test_find_variant(
    const storage_test_variant_t *rows, size_t count, const char *what);

void storage_test_run(storage_test_t *t, const char *phase);
void storage_test_run_all(storage_test_t *t, const char *const *phases);

void storage_test_drop(storage_test_t *t, const char *name,
    enum bridge_on_unload policy);
void storage_test_say_alive(const storage_test_t *t, uint32_t last);

uint64_t storage_test_time_barrier(storage_test_t *t, const char *phase);

uint64_t storage_test_time_systems(storage_test_t *t, const char *phase);

uint64_t storage_test_time_least(storage_test_t *t, const char *phase,
    uint32_t rounds);

void storage_test_time_two(storage_test_t *t, const char *const *phases,
    uint64_t *least);

void storage_test_time_attach_detach(storage_test_t *t,
    storage_test_change_t *c);
void storage_test_time_give_take(storage_test_t *t, storage_test_change_t *c);

void storage_test_say_held(const bridge_world_t *w);

int storage_test_measure_tables(storage_test_t *t);
int storage_test_measure_stress(storage_test_t *t, uint32_t threads);
int storage_test_measure_schedule(storage_test_t *t);

int storage_test_weigh_sets(storage_test_t *t);
int storage_test_weigh_toggles(storage_test_t *t);
int storage_test_weigh_direct(storage_test_t *t);
int storage_test_weigh_tags(storage_test_t *t);
int storage_test_weigh_single(storage_test_t *t);
int storage_test_weigh_switch(storage_test_t *t);

enum storage_test_end storage_test_look_at_shape(storage_test_t *t);
enum storage_test_end storage_test_make_large_matrix(storage_test_t *t);
enum storage_test_end storage_test_make_small_matrix(storage_test_t *t);
enum storage_test_end storage_test_drain_wide_record(storage_test_t *t);
enum storage_test_end storage_test_point_query_away(storage_test_t *t);
enum storage_test_end storage_test_count_types(storage_test_t *t);
enum storage_test_end storage_test_count_all(storage_test_t *t);
enum storage_test_end storage_test_move_many(storage_test_t *t);
enum storage_test_end storage_test_grow_many(storage_test_t *t);
enum storage_test_end storage_test_wrap_generation(storage_test_t *t);
typedef struct {
    bridge_world_t w;
    bridge_system_t *list[BOTH_ROOM];
    declared_type_t types[BOTH_TYPES];
    module_t mods[2 * BOTH_MODULES];
    char fulls[BOTH_MODULES][BOTH_SYSTEMS][16];
    char kinds_of[BOTH_TYPES][8];
    bool current[BOTH_MODULES];
    bool loaded[BOTH_MODULES];
    uint32_t levels[BOTH_ROOM];
    bool marks[BOTH_ROOM][2];
    char why[256];
    uint64_t seed;
    uint32_t builds;
    uint32_t kinds[3];
    uint32_t differ;
} storage_test_both_t;

enum storage_test_end storage_test_go_deep(storage_test_t *t);
enum storage_test_end storage_test_build_both(storage_test_t *t);
uint32_t storage_test_random(storage_test_both_t *b, uint32_t below);
void storage_test_add_module(storage_test_both_t *b, uint32_t record);
void storage_test_drop_module(storage_test_both_t *b, uint32_t record);
void storage_test_name_both(storage_test_both_t *b);
void storage_test_pair_names(storage_test_both_t *b, bridge_order_t *pairs,
    uint32_t count);
int storage_test_build_once(storage_test_both_t *b);
void storage_test_set_parked(storage_test_both_t *b, uint32_t record,
    bool parked);
bridge_system_t *storage_test_make_system(module_t *m, uint32_t terms,
    uint32_t changes, uint32_t words);
int storage_test_build_all(bridge_world_t *w, char *why, size_t why_len);
enum storage_test_end storage_test_run_pool(storage_test_t *t);
enum storage_test_end storage_test_size_hash(storage_test_t *t);
enum storage_test_end storage_test_grow_by_share(storage_test_t *t);
enum storage_test_end storage_test_grow_past(storage_test_t *t);
enum storage_test_end storage_test_ask_less(storage_test_t *t);
enum storage_test_end storage_test_order_two_modules(storage_test_t *t);
enum storage_test_end storage_test_order_phase(storage_test_t *t);
enum storage_test_end storage_test_set_order(storage_test_t *t);
enum storage_test_end storage_test_set_order_guard(storage_test_t *t);
enum storage_test_end storage_test_config_bad_kind(storage_test_t *t);
enum storage_test_end storage_test_relay_bad_kind(storage_test_t *t);
enum storage_test_end storage_test_fill_marks(storage_test_t *t);
enum storage_test_end storage_test_region_full(storage_test_t *t);
enum storage_test_end storage_test_table_full(storage_test_t *t);
enum storage_test_end storage_test_spans_full(storage_test_t *t);
enum storage_test_end storage_test_block_offset(storage_test_t *t);
enum storage_test_end storage_test_index_outside(storage_test_t *t);
enum storage_test_end storage_test_give_two_singles(storage_test_t *t);
enum storage_test_end storage_test_fill_list(storage_test_t *t);
enum storage_test_end storage_test_count_levels(storage_test_t *t);
enum storage_test_end storage_test_count_ranges(storage_test_t *t);
enum storage_test_end storage_test_change(storage_test_t *t);
enum storage_test_end storage_test_make_tables(storage_test_t *t);
enum storage_test_end storage_test_use_up_tables(storage_test_t *t);
enum storage_test_end storage_test_use_up_columns(storage_test_t *t);
enum storage_test_end storage_test_scribble(storage_test_t *t);
enum storage_test_end storage_test_strip(storage_test_t *t);
enum storage_test_end storage_test_strip_full(storage_test_t *t);
enum storage_test_end storage_test_strip_unnamed(storage_test_t *t);
enum storage_test_end storage_test_delete_husk(storage_test_t *t);
enum storage_test_end storage_test_tag(storage_test_t *t);
enum storage_test_end storage_test_toggle(storage_test_t *t);
enum storage_test_end storage_test_toggle_full(storage_test_t *t);
enum storage_test_end storage_test_give_single(storage_test_t *t);
enum storage_test_end storage_test_walk_single(storage_test_t *t);
enum storage_test_end storage_test_attach_single_to_entity(storage_test_t *t);
enum storage_test_end storage_test_attach_to_no_entity(storage_test_t *t);
enum storage_test_end storage_test_index_directly(storage_test_t *t);
enum storage_test_end storage_test_drop_direct(storage_test_t *t);
enum storage_test_end storage_test_join_sparse(storage_test_t *t);
enum storage_test_end storage_test_fill_sparse(storage_test_t *t);
enum storage_test_end storage_test_sweep_beside_sparse(storage_test_t *t);
enum storage_test_end storage_test_drop_sparse(storage_test_t *t);
enum storage_test_end storage_test_relay(storage_test_t *t);
enum storage_test_end storage_test_move(storage_test_t *t);
enum storage_test_end storage_test_stress(storage_test_t *t);
enum storage_test_end storage_test_measure(storage_test_t *t);
enum storage_test_end storage_test_weigh(storage_test_t *t);
enum storage_test_end storage_test_take_baseline(storage_test_t *t);
enum storage_test_end storage_test_live(storage_test_t *t);
enum storage_test_end storage_test_relay_live(storage_test_t *t);
enum storage_test_end storage_test_configure_live(storage_test_t *t);
enum storage_test_end storage_test_group_live(storage_test_t *t);

#endif
