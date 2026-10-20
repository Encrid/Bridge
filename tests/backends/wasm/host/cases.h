/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_CASES_H
    #define TEST_CASES_H

    #include "host.h"

void test_print(void *ud, const bridge_record_t *r);

void test_state_small(bridge_artifact_t *a, char *identity, const char *name);

void test_load_small(host_t *h, const char *name);

void test_state_module(bridge_artifact_t *a, const char *identity,
    const char *file);

void test_load_module(host_t *h, const char *identity, const char *file);

void test_load_mover(host_t *h);

void test_state_reader(bridge_artifact_t *a);

void test_load_reader(host_t *h);

void test_state_reader_limited(bridge_artifact_t *a);

typedef struct {
    uint32_t last_sum[2];
    uint32_t minted[3][256];
    uint32_t minted_count[3];
    int minting;
} test_counts_t;

typedef struct {
    test_counts_t *counts;
    int which;
} test_listener_t;

typedef struct {
    bridge_world_t *w;
    int frames;
    int result;
    host_thread_t thread;
} test_elsewhere_t;

static const char *const TEST_GAME[] = {
    "game:Position",
    "game:Marked"
};

void test_hear_counts(void *ud, const bridge_record_t *r);

test_counts_t *test_get_counts(const host_t *h);

void test_state_wasm(bridge_artifact_t *a, const char *identity,
    const char *file);

void test_load_wasm(host_t *h, const char *identity, const char *file);

void test_state_game_reader(bridge_artifact_t *a);

void test_load_game_reader(host_t *h);

void test_start_elsewhere(test_elsewhere_t *e, bridge_world_t *w, int frames);
void test_join_elsewhere(test_elsewhere_t *e);

typedef struct {
    uint32_t path_at[4];
    uint32_t path_count;
    uint32_t out_of_step;
    bool quiet;
    bridge_setting_t settings[10];
} test_noted_t;

void test_hear_noted(void *ud, const bridge_record_t *r);

test_noted_t *test_get_noted(const host_t *h);

void test_state_wasm_minting(bridge_artifact_t *a, const char *identity,
    const char *file);

void test_state_ticker(bridge_artifact_t *a, const char *identity,
    const char *file);

void test_state_configured(const host_t *h, bridge_artifact_t *a,
    const char *identity);

void test_load_wasm_minting(host_t *h, const char *identity, const char *file);
void test_load_spawnless(host_t *h, const char *identity, const char *file);

void test_reload_settings(host_t *h, bool drop);

void test_say_modules(host_t *h);

typedef struct {
    const char *name;
    size_t at;
    size_t size;
    int64_t value;
} test_edit_t;

typedef bridge_module_config_t test_keys_t;

void test_put_key(void *key, size_t size, int64_t value);
const test_edit_t *test_find_edit(const test_edit_t *rows, const char *name);

typedef struct {
    test_counts_t counts;
    test_listener_t listener;
    test_noted_t noted;
} test_kept_t;

void test_hand_setting(host_t *h);

void test_set_small(host_t *h);
void test_set_wasm(host_t *h);
void test_set_counting(host_t *h);
void test_set_noting(host_t *h);

void test_load_named(host_t *h);
void test_create_no_sink(host_t *h);
void test_stop(host_t *h);
void test_state_no_limit(host_t *h);
void test_state_other_abi(host_t *h);
void test_name_no_backend(host_t *h);
void test_open_no_library(host_t *h);
void test_state_other_host_abi(host_t *h);
void test_state_no_host_abi(host_t *h);
void test_leave_unset(host_t *h);
void test_leave_level_unset(host_t *h);
void test_leave_flag_unset(host_t *h);
void test_state_negative(host_t *h);
void test_state_other_none(host_t *h);
void test_share(host_t *h);
void test_mirror(host_t *h);
void test_declare_other_layout(host_t *h);
void test_declare_other_storage(host_t *h);
void test_declare_ungranted(host_t *h);
void test_reach_direct(host_t *h);
void test_count_toggled(host_t *h);
void test_walk_sparse(host_t *h);
void test_state_bad_term(host_t *h);
void test_undo_load(host_t *h);
void test_lend_scratch(host_t *h);
void test_spend_work(host_t *h);
void test_spawn_to_ceiling(host_t *h);
void test_fill_ring(host_t *h);
void test_overflow_spans(host_t *h);
void test_state_bad_level(host_t *h);
void test_log_warnings(host_t *h);
void test_state_big_private(host_t *h);
void test_state_no_spawns(host_t *h);
void test_run_unknown_phase(host_t *h);
void test_keep_string(host_t *h);
void test_keep_string_no_room(host_t *h);
void test_keep_string_too_big(host_t *h);
void test_leave_phases_unset(host_t *h);
void test_leave_columns_unset(host_t *h);
void test_leave_tables_unset(host_t *h);
void test_leave_types_unset(host_t *h);
void test_leave_systems_unset(host_t *h);
void test_leave_terms_unset(host_t *h);
void test_state_beyond_reach(host_t *h);
void test_state_too_big(host_t *h);
void test_use_up_types(host_t *h);
void test_use_up_systems(host_t *h);
void test_use_up_terms(host_t *h);
void test_use_up_columns(host_t *h);
void test_bench(host_t *h);
void test_state_no_bytes(host_t *h);
void test_order(host_t *h);
void test_reorder(host_t *h);
void test_leave_unordered(host_t *h);
void test_order_in_a_cycle(host_t *h);
void test_order_own(host_t *h);
void test_chain(host_t *h);
void test_take_reserved_name(host_t *h);
void test_state_no_threads(host_t *h);
void test_hop_threads(host_t *h);
void test_tell_events(host_t *h);
void test_nest(host_t *h);
void test_run_two(host_t *h);
void test_mint(host_t *h);
void test_skip_init(host_t *h);
void test_refuse_load(host_t *h);
void test_reach_budget(host_t *h);
void test_stop_by_failing(host_t *h);
void test_give_up(host_t *h);
void test_forgive(host_t *h);
void test_forgive_too_little(host_t *h);
void test_fail_beside_clean(host_t *h);
void test_fail_unlimited(host_t *h);
void test_unload_keeper(host_t *h);
void test_unload_declarer(host_t *h);
void test_destroy_loaded(host_t *h);
void test_unload_nobody(host_t *h);
void test_time_out_shutdown(host_t *h);
void test_take_slot(host_t *h);
void test_grow_share(host_t *h);
void test_repair(host_t *h);
void test_repair_by_load(host_t *h);
void test_revive(host_t *h);
void test_allow_not_reloadable(host_t *h);
void test_leave_reload_out(host_t *h);
void test_reload_nothing(host_t *h);
void test_configure(host_t *h);
void test_misname_setting(host_t *h);
void test_mistype_setting(host_t *h);
void test_overflow_text(host_t *h);
void test_overflow_integer(host_t *h);
void test_overflow_enum(host_t *h);
void test_misname_constant(host_t *h);
void test_state_no_config(host_t *h);
void test_leave_config_unset(host_t *h);
void test_name_missing_config(host_t *h);
void test_state_small_arena(host_t *h);
void test_configure_twice(host_t *h);
void test_reload_configured(host_t *h);
void test_drop_configured(host_t *h);
void test_reseed(host_t *h);
void test_leave_no_room(host_t *h);
void test_configure_table(host_t *h);
void test_churn(host_t *h);
void test_use_arena(host_t *h);
void test_edit_deployment(host_t *h);
void test_edit_phases(host_t *h);
void test_order_nobody(host_t *h);
void test_edit_module(host_t *h);
void test_load_twice(host_t *h);
void test_spawn_past_entities(host_t *h);
void test_edit_phase_names(host_t *h);
void test_do_wrong(host_t *h);
void test_breach(host_t *h);
void test_hoard(host_t *h);
void test_outgrow_spans(host_t *h);
void test_log_levels(host_t *h);
void test_tag_entities(host_t *h);
void test_take_scratch_past(host_t *h);
void test_list_fewer(host_t *h);
void test_refuse_essential(host_t *h);

#endif
