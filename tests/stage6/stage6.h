/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage6/stage6.h -- a host, as far as stage 6: modules that fail,
** stop, leave, come back changed, and are configured. Each case is one
** clause of stage 6 of the plan or one refusal the stage added, checked by
** the records it emits and by what bridge_list_modules answers.
*/

#ifndef TEST_STAGE6_H
    #define TEST_STAGE6_H

    #include "host.h"

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
} stage6_t;

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
} stage6_reload_t;

/*
** stage6_hear
**
** The sink of this program: keeps what the cases count, and prints a
** record unless the program is quiet and it is below a warning.
*/
void stage6_hear(void *ud, const bridge_record_t *r);

/*
** stage6_get
**
** Answers what this program keeps beside the host.
*/
stage6_t *stage6_get(const host_t *h);

/*
** stage6_state_wasm, stage6_state_native
**
** State a module that mints up to 32 ids between two barriers.
*/
void stage6_state_wasm(bridge_artifact_t *a, const char *identity,
    const char *file);
void stage6_state_native(bridge_artifact_t *a, const char *identity,
    const char *path);

/*
** stage6_state_ticker
**
** States a wasm build of the ticker that is reloadable, and may be
** reloaded.
*/
void stage6_state_ticker(bridge_artifact_t *a, const char *identity,
    const char *file);

/*
** stage6_state_configured
**
** States the settings module, with the program's settings and the
** component they seed.
*/
void stage6_state_configured(const host_t *h, bridge_artifact_t *a,
    const char *identity);

/*
** stage6_load_wasm, stage6_load_spawnless
**
** Load a wasm module: as it is stated, or stating that it spawns nothing.
*/
void stage6_load_wasm(host_t *h, const char *identity, const char *file);
void stage6_load_spawnless(host_t *h, const char *identity, const char *file);

/*
** stage6_reload_settings
**
** Loads the settings module and reloads it with another width: into itself,
** or, with `drop`, into a build that declares nothing.
*/
void stage6_reload_settings(host_t *h, bool drop);

/*
** stage6_say_modules
**
** The pull: says what is loaded, and whether it still runs.
*/
void stage6_say_modules(host_t *h);

/*
** The cases. Each is handed the host as main configured it.
*/
void stage6_skip_init(host_t *h);
void stage6_refuse_load(host_t *h);
void stage6_reach_budget(host_t *h);
void stage6_stop_by_failing(host_t *h);
void stage6_give_up(host_t *h);
void stage6_forgive(host_t *h);
void stage6_forgive_too_little(host_t *h);
void stage6_fail_beside_clean(host_t *h);
void stage6_fail_unlimited(host_t *h);
void stage6_unload_keeper(host_t *h);
void stage6_unload_declarer(host_t *h);
void stage6_destroy_loaded(host_t *h);
void stage6_unload_nobody(host_t *h);
void stage6_time_out_shutdown(host_t *h);
void stage6_take_slot(host_t *h);
void stage6_reload_ticker(host_t *h);
void stage6_grow_share(host_t *h);
void stage6_repair(host_t *h);
void stage6_repair_by_load(host_t *h);
void stage6_revive(host_t *h);
void stage6_allow_not_reloadable(host_t *h);
void stage6_leave_reload_out(host_t *h);
void stage6_reload_nothing(host_t *h);
void stage6_state_native_timeout(host_t *h);
void stage6_reload_native(host_t *h);
void stage6_configure(host_t *h);
void stage6_misname_setting(host_t *h);
void stage6_mistype_setting(host_t *h);
void stage6_overflow_text(host_t *h);
void stage6_overflow_integer(host_t *h);
void stage6_overflow_enum(host_t *h);
void stage6_misname_constant(host_t *h);
void stage6_state_no_config(host_t *h);
void stage6_leave_config_unset(host_t *h);
void stage6_name_missing_config(host_t *h);
void stage6_state_small_arena(host_t *h);
void stage6_configure_twice(host_t *h);
void stage6_reload_configured(host_t *h);
void stage6_drop_configured(host_t *h);
void stage6_reseed(host_t *h);
void stage6_leave_no_room(host_t *h);
void stage6_configure_table(host_t *h);
void stage6_churn(host_t *h);
void stage6_use_arena(host_t *h);

#endif
