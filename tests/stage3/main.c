/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage3/main.c -- stage3 <case>: the cases of the program, by name.
*/

#include "stage3.h"

static const host_case_t STAGE3_CASES[] = {
    { "share", stage3_share },
    { "spans", stage3_share },
    { "log", stage3_share },
    { "mirror", stage3_mirror },
    { "keptunset", stage3_declare_other_storage },
    { "direct", stage3_reach_direct },
    { "toggle", stage3_count_toggled },
    { "sparse", stage3_walk_sparse },
    { "fingerprint", stage3_declare_other_layout },
    { "grant", stage3_declare_ungranted },
    { "badterm", stage3_state_bad_term },
    { "smallscratch", stage3_lend_scratch },
    { "worklimit", stage3_spend_work },
    { "ceiling", stage3_spawn_to_ceiling },
    { "ring", stage3_fill_ring },
    { "nospans", stage3_overflow_spans },
    { "nocolumns", stage3_use_up_columns },
    { "nophase", stage3_run_unknown_phase },
    { "nophases", stage3_leave_phases_unset },
    { "unsettables", stage3_leave_tables_unset },
    { "unsettypes", stage3_leave_types_unset },
    { "unsetsystems", stage3_leave_systems_unset },
    { "notypes", stage3_use_up_types },
    { "nosystems", stage3_use_up_systems },
    { "loglevel", stage3_state_bad_level },
    { "quiet", stage3_log_warnings },
    { "unsetterms", stage3_leave_terms_unset },
    { "noterms", stage3_use_up_terms },
    { "nobudget", stage3_leave_columns_unset },
    { "beyond", stage3_state_beyond_reach },
    { "toobig", stage3_state_too_big },
    { "bigprivate", stage3_state_big_private },
    { "bench", stage3_bench },
    { "strings", stage3_keep_string },
    { "noarena", stage3_keep_string_no_room },
    { "bigarena", stage3_keep_string_too_big },
    { "nospawns", stage3_state_no_spawns },
    { "undo", stage3_undo_load }
};

/*
** main
**
** Runs the case the command line names, on a host with the wasm backend and
** both phases.
*/
int main(int argc, char **argv)
{
    constexpr size_t count = sizeof(STAGE3_CASES) / sizeof(*STAGE3_CASES);
    host_t h;

    host_init(&h, argc, argv);
    h.cfg.backend_count = 1;
    return (host_run(&h, STAGE3_CASES, count));
}
