/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage2/main.c -- stage2 <case>: the cases of the program, by name.
*/

#include "stage2.h"

static const host_case_t STAGE2_CASES[] = {
    { "ok", stage2_load_named },
    { "image", stage2_load_named },
    { "oob", stage2_load_named },
    { "spin", stage2_load_named },
    { "grow", stage2_load_named },
    { "big", stage2_load_named },
    { "shape", stage2_load_named },
    { "shape64", stage2_load_named },
    { "noinit", stage2_load_named },
    { "nolimit", stage2_state_no_limit },
    { "abi", stage2_state_other_abi },
    { "nobackend", stage2_name_no_backend },
    { "unsetlevel", stage2_leave_level_unset },
    { "unset", stage2_leave_unset },
    { "nonelimit", stage2_state_other_none },
    { "hostabi", stage2_state_other_host_abi },
    { "hostabiunset", stage2_state_no_host_abi },
    { "unsetflag", stage2_leave_flag_unset },
    { "negative", stage2_state_negative },
    { "stop", stage2_stop },
    { "nolib", stage2_open_no_library },
    { "nosink", stage2_create_no_sink }
};

/*
** main
**
** Runs the case the command line names, on a host with the wasm backend and
** the one phase "update".
*/
int main(int argc, char **argv)
{
    constexpr size_t count = sizeof(STAGE2_CASES) / sizeof(*STAGE2_CASES);
    host_t h;

    host_init(&h, argc, argv);
    h.cfg.backend_count = 1;
    h.cfg.phase_count = 1;
    h.sink = stage2_print;
    return (host_run(&h, STAGE2_CASES, count));
}
