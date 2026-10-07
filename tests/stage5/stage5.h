/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage5/stage5.h -- a host, as far as stage 5: order, observers,
** threads. Each case is one clause of stage 5 of the plan, checked by the
** records it emits. A record is printed with the barrier count it was
** written under, since two of the claims are about which barrier something
** happened at.
*/

#ifndef TEST_STAGE5_H
    #define TEST_STAGE5_H

    #include "host.h"

/*
** What the sink of this program counts, of the records it does not print.
**
**     threads, began, ended
**                   the thread a napping module said it ran on, and when
**                   each nap began and ended, in milliseconds
**     pins, pins_here
**                   the calls a pinned module said it had, and how many of
**                   them were on the thread that loaded it
**     last_sum      of two bridges with one sink, the last sum each said
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
} stage5_counts_t;

/*
** Who a bridge's records are from: the counts, and which of two bridges it
** is, or -1 where there is one.
*/
typedef struct {
    stage5_counts_t *counts;
    int which;
} stage5_listener_t;

/*
** A call made from a thread that loaded nothing: bridge_destroy if `frames`
** is negative, one "update" if it is zero, and otherwise that many frames.
*/
typedef struct {
    bridge_t *b;
    int frames;
    int result;
    host_thread_t thread;
} stage5_elsewhere_t;

/*
** The two components of the game namespace, which a module of another one
** is granted.
*/
static const char *const STAGE5_GAME[] = {
    "game:Position",
    "game:Marked"
};

/*
** stage5_hear
**
** The sink of this program: keeps what the cases count, and prints every
** other record with its barrier.
*/
void stage5_hear(void *ud, const bridge_record_t *r);

/*
** stage5_get_counts
**
** Answers what the host's sink counts.
*/
stage5_counts_t *stage5_get_counts(const host_t *h);

/*
** stage5_count_threads
**
** Answers how many distinct threads the napping modules said they ran on.
*/
uint32_t stage5_count_threads(const stage5_counts_t *c);

/*
** stage5_state_wasm
**
** States a wasm module of tests/modules, with what each mints between two
** barriers: most mint nothing, and say so.
*/
void stage5_state_wasm(bridge_artifact_t *a, const char *identity,
    const char *file);

/*
** stage5_load_wasm, stage5_load_native
**
** Load a module as it is stated, a native one minting `spawns` ids.
*/
void stage5_load_wasm(host_t *h, const char *identity, const char *file);
void stage5_load_native(host_t *h, const char *identity, const char *path,
    int64_t spawns);

/*
** stage5_state_reader
**
** States the reader, with the grant to read the two components of the game
** namespace.
*/
void stage5_state_reader(bridge_artifact_t *a);

/*
** stage5_load_reader, stage5_load_watcher
**
** Load the reader, or the watcher, each with that grant.
*/
void stage5_load_reader(host_t *h);
void stage5_load_watcher(host_t *h);

/*
** stage5_start_elsewhere, stage5_join_elsewhere
**
** Make a call from another thread, and wait for it.
*/
void stage5_start_elsewhere(stage5_elsewhere_t *e, bridge_t *b, int frames);
void stage5_join_elsewhere(stage5_elsewhere_t *e);

/*
** The cases. Each is handed the host as main configured it.
*/
void stage5_order(host_t *h);
void stage5_reorder(host_t *h);
void stage5_leave_unordered(host_t *h);
void stage5_order_in_a_cycle(host_t *h);
void stage5_order_own(host_t *h);
void stage5_observe(host_t *h);
void stage5_chain(host_t *h);
void stage5_take_reserved_name(host_t *h);
void stage5_run_at_once(host_t *h);
void stage5_run_in_turn(host_t *h);
void stage5_enter_from_foreign_thread(host_t *h);
void stage5_pin(host_t *h);
void stage5_call_from_wrong_thread(host_t *h);
void stage5_state_no_threads(host_t *h);
void stage5_hop_threads(host_t *h);
void stage5_tell_events(host_t *h);
void stage5_conflict_at_barrier(host_t *h);
void stage5_order_observers(host_t *h);
void stage5_reorder_observers(host_t *h);
void stage5_queue_too_few(host_t *h);
void stage5_queue_none(host_t *h);
void stage5_leave_phase_unmapped(host_t *h);
void stage5_map_phase(host_t *h);
void stage5_map_phase_nowhere(host_t *h);
void stage5_change_unstated(host_t *h);
void stage5_conflict_by_change(host_t *h);
void stage5_order_changes(host_t *h);
void stage5_order_changes_loaded_last(host_t *h);
void stage5_tell_both(host_t *h);
void stage5_reap_with_read(host_t *h);
void stage5_reap_with_write(host_t *h);
void stage5_nest(host_t *h);
void stage5_run_two(host_t *h);
void stage5_mint(host_t *h);

#endif
