/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage3/stage3.h -- a host, as far as stage 3: it creates a bridge
** with storage, loads modules compiled from C, runs phases, and prints
** every record. Each case is one clause of stage 3 of the plan. build.py
** checks a case by the records it emits, and every case ends by printing
** "alive".
*/

#ifndef TEST_STAGE3_H
    #define TEST_STAGE3_H

    #include "host.h"

/*
** stage3_state
**
** States the wasm module of a file under an identity, with what each test
** module mints between two barriers: most mint nothing, and say so. The
** rows modules attach 200,000 rows in one init, at 32 bytes a change.
*/
void stage3_state(bridge_artifact_t *a, const char *identity,
    const char *file);

/*
** stage3_load
**
** Loads the wasm module of a file under an identity.
*/
void stage3_load(host_t *h, const char *identity, const char *file);

/*
** stage3_load_mover
**
** Loads the mover, which owns game:Position and spawns the entities.
*/
void stage3_load_mover(host_t *h);

/*
** stage3_state_reader
**
** States the reader, of another namespace, with the grant to read the two
** components of the mover.
*/
void stage3_state_reader(bridge_artifact_t *a);

/*
** stage3_load_reader
**
** Loads the reader with its grant.
*/
void stage3_load_reader(host_t *h);

/*
** The cases. Each is handed the host as main configured it.
*/
void stage3_share(host_t *h);
void stage3_mirror(host_t *h);
void stage3_declare_other_layout(host_t *h);
void stage3_declare_other_storage(host_t *h);
void stage3_declare_ungranted(host_t *h);
void stage3_reach_direct(host_t *h);
void stage3_count_toggled(host_t *h);
void stage3_walk_sparse(host_t *h);
void stage3_state_bad_term(host_t *h);
void stage3_undo_load(host_t *h);
void stage3_lend_scratch(host_t *h);
void stage3_spend_work(host_t *h);
void stage3_spawn_to_ceiling(host_t *h);
void stage3_fill_ring(host_t *h);
void stage3_overflow_spans(host_t *h);
void stage3_state_bad_level(host_t *h);
void stage3_log_warnings(host_t *h);
void stage3_state_big_private(host_t *h);
void stage3_state_no_spawns(host_t *h);
void stage3_run_unknown_phase(host_t *h);
void stage3_keep_string(host_t *h);
void stage3_keep_string_no_room(host_t *h);
void stage3_keep_string_too_big(host_t *h);
void stage3_leave_phases_unset(host_t *h);
void stage3_leave_columns_unset(host_t *h);
void stage3_leave_tables_unset(host_t *h);
void stage3_leave_types_unset(host_t *h);
void stage3_leave_systems_unset(host_t *h);
void stage3_leave_terms_unset(host_t *h);
void stage3_state_beyond_reach(host_t *h);
void stage3_state_too_big(host_t *h);
void stage3_use_up_types(host_t *h);
void stage3_use_up_systems(host_t *h);
void stage3_use_up_terms(host_t *h);
void stage3_use_up_columns(host_t *h);
void stage3_bench(host_t *h);

#endif
