/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/cases.h -- the host program: a host that links the library as
** a host would, and its cases. build.py checks a refusal by the record it
** emits, not by a return code alone, and every case ends by printing
** "alive".
*/

#ifndef TEST_CASES_H
    #define TEST_CASES_H

    #include "host.h"

/*
** test_print
**
** The sink of this program: prints a record's level and its text.
*/
void test_print(void *ud, const bridge_record_t *r);

/*
** test_state_small
**
** States the wasm module of a name as `test:<name>@0.1.0`, with the small
** limits this stage loads everything under.
*/
void test_state_small(bridge_artifact_t *a, char *identity, const char *name);

/*
** test_load_small
**
** Loads the wasm module of a name, stated as test_state_small has it.
*/
void test_load_small(host_t *h, const char *name);

/*
** test_state_module
**
** States the wasm module of a file under an identity, with what each test
** module mints between two barriers: most mint nothing, and say so. The
** rows modules attach 200,000 rows in one init, at 32 bytes a change.
*/
void test_state_module(bridge_artifact_t *a, const char *identity,
    const char *file);

/*
** test_load_module
**
** Loads the wasm module of a file under an identity.
*/
void test_load_module(host_t *h, const char *identity, const char *file);

/*
** test_load_mover
**
** Loads the mover, which owns game:Position and spawns the entities.
*/
void test_load_mover(host_t *h);

/*
** test_state_reader
**
** States the reader, of another namespace, with the grant to read the two
** components of the mover.
*/
void test_state_reader(bridge_artifact_t *a);

/*
** test_load_reader
**
** Loads the reader with its grant.
*/
void test_load_reader(host_t *h);

/*
** test_state_native
**
** States a native module that mints up to 1,024 ids between two barriers.
*/
void test_state_native(bridge_artifact_t *a, const char *identity,
    const char *path);

/*
** test_state_reader_limited
**
** States the reader of the mover, on the wasm backend, with the limits that
** backend can keep and its grant. It only reads.
*/
void test_state_reader_limited(bridge_artifact_t *a);

/*
** What the sink of this program counts, of the records it does not print.
**
**     threads, began, ended
**                   the thread a napping module said it ran on, and when
**                   each nap began and ended, in milliseconds
**     pins, pins_here
**                   the calls a pinned module said it had, and how many of
**                   them were on the thread that loaded it
**     last_sum      of two decks with one sink, the last sum each said
**     minted        the ids a run's modules were given, in the order the
**                   records arrived, for each of three runs
**     minting       which run is in progress, or -1
*/
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

/*
** Who a deck's records are from: the counts, and which of two decks it
** is, or -1 where there is one.
*/
typedef struct {
    test_counts_t *counts;
    int which;
} test_listener_t;

/*
** A call made from a thread that loaded nothing: bridge_destroy if `frames`
** is negative, one "update" if it is zero, and otherwise that many frames.
*/
typedef struct {
    bridge_t *b;
    int frames;
    int result;
    host_thread_t thread;
} test_elsewhere_t;

/*
** The two components of the game namespace, which a module of another one
** is granted.
*/
static const char *const TEST_GAME[] = {
    "game:Position",
    "game:Marked"
};

/*
** test_hear_counts
**
** The sink of this program: keeps what the cases count, and prints every
** other record with its barrier.
*/
void test_hear_counts(void *ud, const bridge_record_t *r);

/*
** test_get_counts
**
** Answers what the host's sink counts.
*/
test_counts_t *test_get_counts(const host_t *h);

/*
** test_count_threads
**
** Answers how many distinct threads the napping modules said they ran on.
*/
uint32_t test_count_threads(const test_counts_t *c);

/*
** test_state_wasm
**
** States a wasm module of tests/modules, with what each mints between two
** barriers: most mint nothing, and say so.
*/
void test_state_wasm(bridge_artifact_t *a, const char *identity,
    const char *file);

/*
** test_load_wasm, test_load_native
**
** Load a module as it is stated, a native one minting `spawns` ids.
*/
void test_load_wasm(host_t *h, const char *identity, const char *file);
void test_load_built(host_t *h, const char *identity, const char *name,
    int64_t spawns);
void test_load_native(host_t *h, const char *identity, const char *path,
    int64_t spawns);

/*
** test_state_game_reader
**
** States the reader, with the grant to read the two components of the game
** namespace.
*/
void test_state_game_reader(bridge_artifact_t *a);

/*
** test_load_game_reader, test_load_watcher
**
** Load the reader, or the watcher, each with that grant.
*/
void test_load_game_reader(host_t *h);
void test_load_watcher(host_t *h);

/*
** test_start_elsewhere, test_join_elsewhere
**
** Make a call from another thread, and wait for it.
*/
void test_start_elsewhere(test_elsewhere_t *e, bridge_t *b, int frames);
void test_join_elsewhere(test_elsewhere_t *e);

/*
** What this program keeps beside the host.
**
**     path_at       where the settings module said its text was, each time
**     out_of_step   how many records said the entity index is out of step
**     quiet         only warnings and errors are printed
**     settings      a module's settings, of every kind, as a deployment
**                   would hand them over: a case changes one before it
**                   loads
*/
typedef struct {
    uint32_t path_at[4];
    uint32_t path_count;
    uint32_t out_of_step;
    bool quiet;
    bridge_setting_t settings[10];
} test_noted_t;

/*
** How a case of the reload family reloads the ticker.
**
**     file          the wasm build it is reloaded into, or nullptr for the
**                   native one
**     may           what the deployment's hot_reload says
**     spawns        what the new instance states it mints
**     then          a module loaded afterwards, or nullptr
*/
typedef struct {
    const char *name;
    const char *file;
    enum bridge_flag may;
    int64_t spawns;
    const char *then;
} test_reload_t;

/*
** test_hear_noted
**
** The sink of this program: keeps what the cases count, and prints a
** record unless the program is quiet and it is below a warning.
*/
void test_hear_noted(void *ud, const bridge_record_t *r);

/*
** test_get_noted
**
** Answers what this program keeps beside the host.
*/
test_noted_t *test_get_noted(const host_t *h);

/*
** test_state_wasm_minting, test_state_native_minting
**
** State a module that mints up to 32 ids between two barriers.
*/
void test_state_wasm_minting(bridge_artifact_t *a, const char *identity,
    const char *file);
void test_state_native_minting(bridge_artifact_t *a, const char *identity,
    const char *path);

/*
** test_state_ticker
**
** States a wasm build of the ticker that is reloadable, and may be
** reloaded.
*/
void test_state_ticker(bridge_artifact_t *a, const char *identity,
    const char *file);

/*
** test_state_configured
**
** States the settings module, with the program's settings and the
** component they seed.
*/
void test_state_configured(const host_t *h, bridge_artifact_t *a,
    const char *identity);

/*
** test_load_wasm_minting, test_load_spawnless
**
** Load a wasm module: as it is stated, or stating that it spawns nothing.
*/
void test_load_wasm_minting(host_t *h, const char *identity, const char *file);
void test_load_spawnless(host_t *h, const char *identity, const char *file);

/*
** test_reload_settings
**
** Loads the settings module and reloads it with another width: into itself,
** or, with `drop`, into a build that declares nothing.
*/
void test_reload_settings(host_t *h, bool drop);

/*
** test_say_modules
**
** The pull: says what is loaded, and whether it still runs.
*/
void test_say_modules(host_t *h);

/*
** A key stated as what a case says: its row's name, where the key is in
** the struct, how many bytes it is, and the value it is given.
*/
typedef struct {
    const char *name;
    size_t at;
    size_t size;
    int64_t value;
} test_edit_t;

/*
** The struct a module's keys are in, by a name that leaves a row of edits
** its line.
*/
typedef bridge_module_config_t test_keys_t;

/*
** test_put_key, test_find_edit
**
** Write a value into a key of `size` bytes, and answer the row of a name
** among rows that hold it.
*/
void test_put_key(void *key, size_t size, int64_t value);
const test_edit_t *test_find_edit(const test_edit_t *rows, const char *name);

/*
** What the fixtures keep beside the host, for the sinks that count.
*/
typedef struct {
    test_counts_t counts;
    test_listener_t listener;
    test_noted_t noted;
} test_kept_t;

/*
** test_set_small, test_set_wasm, test_set_both, test_set_counting,
** test_set_noting
**
** The fixtures: what a case's host is before the case changes it. The wasm
** backend with the one phase "update" and the sink that prints a level;
** the wasm backend with both phases; both backends with both phases; and
** that again with the sink that counts, and with the sink that notes.
*/
void test_set_small(host_t *h);
void test_set_wasm(host_t *h);
void test_set_both(host_t *h);
void test_set_counting(host_t *h);
void test_set_noting(host_t *h);

/*
** The cases. Each is handed the host as its fixture set it.
*/
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
void test_run_native(host_t *h);
void test_run_mixed(host_t *h);
void test_state_untrusted(host_t *h);
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
void test_order_own(host_t *h);
void test_observe(host_t *h);
void test_chain(host_t *h);
void test_take_reserved_name(host_t *h);
void test_run_at_once(host_t *h);
void test_run_in_turn(host_t *h);
void test_enter_from_foreign_thread(host_t *h);
void test_pin(host_t *h);
void test_call_from_wrong_thread(host_t *h);
void test_state_no_threads(host_t *h);
void test_hop_threads(host_t *h);
void test_tell_events(host_t *h);
void test_conflict_at_barrier(host_t *h);
void test_order_observers(host_t *h);
void test_reorder_observers(host_t *h);
void test_queue_too_few(host_t *h);
void test_queue_none(host_t *h);
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
void test_unload_keeper(host_t *h);
void test_unload_declarer(host_t *h);
void test_destroy_loaded(host_t *h);
void test_unload_nobody(host_t *h);
void test_time_out_shutdown(host_t *h);
void test_take_slot(host_t *h);
void test_reload_ticker(host_t *h);
void test_grow_share(host_t *h);
void test_repair(host_t *h);
void test_repair_by_load(host_t *h);
void test_revive(host_t *h);
void test_allow_not_reloadable(host_t *h);
void test_leave_reload_out(host_t *h);
void test_reload_nothing(host_t *h);
void test_state_native_timeout(host_t *h);
void test_reload_native(host_t *h);
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
void test_edit_backend(host_t *h);
void test_edit_phases(host_t *h);
void test_order_nobody(host_t *h);
void test_edit_module(host_t *h);
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

#endif
