/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage5/main.c -- stage5 <case>: the cases of the program, by name.
*/

#include "stage5.h"

static const host_case_t STAGE5_CASES[] = {
    { "order", stage5_order },
    { "reorder", stage5_reorder },
    { "ambiguous", stage5_leave_unordered },
    { "ownorder", stage5_order_own },
    { "cycle", stage5_order_in_a_cycle },
    { "observe", stage5_observe },
    { "chain", stage5_chain },
    { "reserved", stage5_take_reserved_name },
    { "parallel", stage5_run_at_once },
    { "serial", stage5_run_in_turn },
    { "foreign", stage5_enter_from_foreign_thread },
    { "pinned", stage5_pin },
    { "wrongthread", stage5_call_from_wrong_thread },
    { "nothreads", stage5_state_no_threads },
    { "hop", stage5_hop_threads },
    { "events", stage5_tell_events },
    { "obsconflict", stage5_conflict_at_barrier },
    { "fewchanges", stage5_queue_too_few },
    { "nochanges", stage5_queue_none },
    { "phaseunmapped", stage5_leave_phase_unmapped },
    { "phasemapped", stage5_map_phase },
    { "phasebadmap", stage5_map_phase_nowhere },
    { "unstated", stage5_change_unstated },
    { "changeconflict", stage5_conflict_by_change },
    { "changeorder", stage5_order_changes },
    { "changeloaded", stage5_order_changes_loaded_last },
    { "reapread", stage5_reap_with_read },
    { "reapwrite", stage5_reap_with_write },
    { "bothtold", stage5_tell_both },
    { "obsorder", stage5_order_observers },
    { "obsreorder", stage5_reorder_observers },
    { "nested", stage5_nest },
    { "nested2", stage5_nest },
    { "two", stage5_run_two },
    { "minted", stage5_mint }
};

/*
** main
**
** Runs the case the command line names, on a host with both backends and
** both phases, whose sink counts.
*/
int main(int argc, char **argv)
{
    constexpr size_t count = sizeof(STAGE5_CASES) / sizeof(*STAGE5_CASES);
    stage5_counts_t counts = { .minting = -1 };
    stage5_listener_t listener = { &counts, -1 };
    host_t h;

    host_init(&h, argc, argv);
    h.sink = stage5_hear;
    h.ud = &listener;
    return (host_run(&h, STAGE5_CASES, count));
}
