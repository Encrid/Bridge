/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage4/main.c -- stage4 <case>: the cases of the program, by name.
*/

#include "stage4.h"

static const host_case_t STAGE4_CASES[] = {
    { "raw", stage4_run_native },
    { "mixed", stage4_run_mixed },
    { "untrusted", stage4_state_untrusted },
    { "limit", stage4_state_limit },
    { "libraries", stage4_leave_libraries },
    { "backendabi", stage4_open_other_seam },
    { "nolib", stage4_name_no_library },
    { "refuse", stage4_refuse_entry },
    { "setting", stage4_state_unknown_setting },
    { "nopath", stage4_state_no_path },
    { "nobytes", stage4_state_no_bytes }
};

/*
** main
**
** Runs the case the command line names, on a host with both backends and
** both phases.
*/
int main(int argc, char **argv)
{
    constexpr size_t count = sizeof(STAGE4_CASES) / sizeof(*STAGE4_CASES);
    host_t h;

    host_init(&h, argc, argv);
    return (host_run(&h, STAGE4_CASES, count));
}
