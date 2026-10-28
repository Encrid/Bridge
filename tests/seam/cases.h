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

void test_state_plain(bridge_artifact_t *a, const char *identity,
    const char *path);

void test_state_reader_limited(bridge_artifact_t *a);

typedef struct {
    uint32_t threads[64];
    uint32_t began[64];
    uint32_t ended[64];
    uint32_t seen;
    uint32_t ended_count;
    uint32_t pins;
    uint32_t pins_here;
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

static const char *const TEST_TICK_LOSS[] = {
    "game:Tick.n"
};

static const bridge_setting_t TEST_STRAY[] = {
    { .key = "stray", .kind = BRIDGE_INT, .v = { .i = 1 } }
};

void test_hear_counts(void *ud, const bridge_record_t *r);

test_counts_t *test_get_counts(const host_t *h);

uint32_t test_count_threads(const test_counts_t *c);

void test_state_guarded(bridge_artifact_t *a, const char *identity,
    const char *file);

void test_load_guarded(host_t *h, const char *identity, const char *file);
void test_load_built(host_t *h, const char *identity, const char *name,
    int64_t spawns);
void test_state_built_minting(bridge_artifact_t *a, const char *identity,
    const char *name);
void test_load_plain(host_t *h, const char *identity, const char *path,
    int64_t spawns);

void test_state_game_reader(bridge_artifact_t *a);

void test_load_game_reader(host_t *h);
void test_load_watcher(host_t *h);

void test_start_elsewhere(test_elsewhere_t *e, bridge_world_t *w, int frames);
void test_join_elsewhere(test_elsewhere_t *e);

typedef struct {
    uint32_t path_at[4];
    uint32_t path_count;
    uint32_t out_of_step;
    bool quiet;
    bridge_setting_t settings[10];
} test_noted_t;

typedef struct {
    const char *name;
    const char *file;
    enum bridge_flag may;
    int64_t spawns;
    const char *then;
} test_reload_t;

void test_hear_noted(void *ud, const bridge_record_t *r);

test_noted_t *test_get_noted(const host_t *h);

void test_state_guarded_minting(bridge_artifact_t *a, const char *identity,
    const char *file);
void test_state_plain_minting(bridge_artifact_t *a, const char *identity,
    const char *path);

void test_state_ticker(bridge_artifact_t *a, const char *identity,
    const char *file);

void test_state_configured(const host_t *h, bridge_artifact_t *a,
    const char *identity);

void test_load_guarded_minting(host_t *h, const char *identity,
    const char *file);
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

void test_set_small(host_t *h);
void test_set_guarded(host_t *h);
void test_set_both(host_t *h);
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
void test_declare_other_on_destroy(host_t *h);
void test_declare_ungranted(host_t *h);
void test_reach_direct(host_t *h);

void test_declare_huge(host_t *h);

void test_grow_past_room(host_t *h);
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
void test_run_plain(host_t *h);
void test_run_mixed(host_t *h);
void test_state_limit(host_t *h);
void test_state_no_path(host_t *h);
void test_state_no_bytes(host_t *h);
void test_name_no_library(host_t *h);
void test_refuse_entry(host_t *h);
void test_leave_libraries(host_t *h);
void test_open_other_seam(host_t *h);
void test_state_unknown_setting(host_t *h);
void test_order(host_t *h);
void test_reorder(host_t *h);
void test_leave_unordered(host_t *h);
void test_order_in_a_cycle(host_t *h);
void test_order_pair_against_before(host_t *h);
void test_order_pair_beside_before(host_t *h);
void test_reload_into_cycle(host_t *h);

void test_order_unnamed(host_t *h);

void test_order_chain_unloaded(host_t *h);

void test_order_through_a_module(host_t *h);

void test_load_by_identity(host_t *h);
void test_order_own(host_t *h);
void test_observe(host_t *h);
void test_chain(host_t *h);
void test_take_reserved_name(host_t *h);
void test_run_at_once(host_t *h);
void test_run_in_turn(host_t *h);
void test_observe_in_turn(host_t *h);
void test_run_own_in_turn(host_t *h);
void test_enter_from_foreign_thread(host_t *h);
void test_pin(host_t *h);
void test_call_from_wrong_thread(host_t *h);

void test_set_calling(host_t *h);

void test_set_calling_elsewhere(host_t *h);

void test_call_from_sink(host_t *h);

void test_call_after_sink(host_t *h);
void test_state_no_threads(host_t *h);
void test_hop_threads(host_t *h);
void test_tell_events(host_t *h);

void test_run_guarded(host_t *h, const char *identity, const char *file,
    int frames);

void test_tell_places(host_t *h);
void test_map_onto_a_named_phase(host_t *h);
void test_map_beside_a_named_phase(host_t *h);
void test_place_before(host_t *h);
void test_place_paired(host_t *h);
void test_place_two(host_t *h);
void test_place_reordered(host_t *h);

void test_take_in_old_shutdown(host_t *h);

void test_guess(host_t *h);

void test_guess_later(host_t *h);

void test_stick(host_t *h);

void test_sweep(host_t *h);

void test_round_spans(host_t *h);

void test_alloc_unaligned_share(host_t *h);

void test_reload_beyond_reach(host_t *h);

void test_stop_by_spans(host_t *h);

void test_drain_failed_start(host_t *h);
void test_shut_failed_load(host_t *h);

void test_keep_after_failed_reload(host_t *h);

void test_watch_no_room(host_t *h);

void test_name_many_declarers(host_t *h);

void test_two_turns_of_one_module(host_t *h);
void test_walk_optional(host_t *h);

void test_pick_sparse(host_t *h);

void test_hear_singleton_twice(host_t *h);

void test_call_with_nulls(host_t *h);

void test_hot_reload_unkept(host_t *h);

void test_repair_fails_again(host_t *h);

void test_reload_into_conflict(host_t *h);

void test_reload_then_stop(host_t *h);

void test_reload_colliding(host_t *h);

void test_load_rival(host_t *h);

void test_declare_other_size(host_t *h);

void test_load_long_identity(host_t *h);

void test_reload_settings_layout(host_t *h);

void test_keep_two_shares(host_t *h);

void test_keep_for_another(host_t *h);
void test_keep_text_refused(host_t *h);

void test_seed_failed_load(host_t *h);
void test_config_refused_under_fail(host_t *h);

void test_hand_keys(host_t *h);

void test_state_wide(host_t *h);

void test_stop_by_barred(host_t *h);

void test_stop_two_observers(host_t *h);

void test_mend_after_fault(host_t *h);

void test_order_two_pairs(host_t *h);
void test_order_no_array(host_t *h);
void test_order_too_many(host_t *h);

void test_log_out_of_range(host_t *h);

void test_reserve_too_much(host_t *h);

void test_state_shape(host_t *h);
void test_conflict_at_barrier(host_t *h);
void test_conflict_own_observers(host_t *h);
void test_conflict_changes_at_barrier(host_t *h);
void test_order_observers(host_t *h);
void test_reorder_observers(host_t *h);
void test_queue_too_few(host_t *h);
void test_queue_none(host_t *h);
void test_read_room(host_t *h);
void test_leave_phase_unmapped(host_t *h);
void test_map_phase(host_t *h);
void test_map_phase_nowhere(host_t *h);
void test_change_unstated(host_t *h);
void test_conflict_by_change(host_t *h);
void test_order_changes(host_t *h);
void test_order_changes_loaded_last(host_t *h);
void test_tell_both(host_t *h);
void test_reap_with_read(host_t *h);
void test_reap_with_write(host_t *h);
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
void test_end_by_system(host_t *h);
void test_end_by_init(host_t *h);
void test_end_by_observer(host_t *h);
void test_end_at_a_reload(host_t *h);
void test_unload_keeper(host_t *h);
void test_make_husk(host_t *h);
void test_reload_husk(host_t *h);
void test_unload_husk(host_t *h);
void test_crowd_husk(host_t *h);
void test_unload_declarer(host_t *h);
void test_destroy_loaded(host_t *h);
void test_unload_nobody(host_t *h);
void test_time_out_shutdown(host_t *h);
void test_time_out_failed_load(host_t *h);
void test_take_slot(host_t *h);
void test_reload_ticker(host_t *h);
void test_reload_tag(host_t *h);
void test_reload_while_stopped(host_t *h);

void group_state(bridge_artifact_t *a, const char *identity, const char *file);
void test_reload_group(host_t *h);
void test_reload_group_outside(host_t *h);
void test_reload_group_start(host_t *h);
void test_reload_group_cycle(host_t *h);
void test_reload_group_plain(host_t *h);
void test_reload_group_empty(host_t *h);
void test_reload_group_dup(host_t *h);
void test_reload_group_ghost(host_t *h);
void test_reload_group_stop(host_t *h);
void test_reload_group_hold(host_t *h);
void test_reload_group_shut(host_t *h);
void test_grow_share(host_t *h);
void test_hoard_shares(host_t *h);
void test_repair(host_t *h);
void test_repair_by_load(host_t *h);
void test_revive(host_t *h);
void test_allow_not_reloadable(host_t *h);
void test_leave_reload_out(host_t *h);

void test_leave_shutdown_out(host_t *h);

void test_scan_shares(host_t *h);
void test_reload_nothing(host_t *h);
void test_state_plain_timeout(host_t *h);
void test_reload_plain(host_t *h);
void test_reload_plain_loss(host_t *h);
void test_configure(host_t *h);
void test_spoil_config(host_t *h);
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
void test_edit_backend(host_t *h);
void test_edit_phases(host_t *h);
void test_order_nobody(host_t *h);
void test_edit_module(host_t *h);

void test_edit_module_more(host_t *h);

void test_map_phase_second(host_t *h);
void test_map_two_phases(host_t *h);
void test_load_twice(host_t *h);
void test_spawn_past_entities(host_t *h);
void test_edit_phase_names(host_t *h);
void test_do_wrong(host_t *h);
void test_outgrow_spans(host_t *h);
void test_log_levels(host_t *h);
void test_tag_entities(host_t *h);
void test_take_scratch_past(host_t *h);
void test_list_fewer(host_t *h);
void test_hand_settings(host_t *h);
void test_refuse_essential(host_t *h);

void test_profile(host_t *h);
void test_set_profile(host_t *h);

void test_usage(host_t *h);
void test_set_usage(host_t *h);

void test_scribble(host_t *h);
void test_set_validate(host_t *h);

#endif
