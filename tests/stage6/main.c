/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage6/main.c -- stage6 <case>: the cases of the program, by name.
*/

#include "stage6.h"

static const host_case_t STAGE6_CASES[] = {
    { "skipinit", stage6_skip_init },
    { "refusedload", stage6_refuse_load },
    { "budget", stage6_reach_budget },
    { "forgiven", stage6_forgive },
    { "notforgiven", stage6_forgive_too_little },
    { "notmasked", stage6_fail_beside_clean },
    { "neverstop", stage6_fail_unlimited },
    { "failstop", stage6_stop_by_failing },
    { "failgiveup", stage6_give_up },
    { "strip", stage6_unload_keeper },
    { "sweep", stage6_unload_keeper },
    { "delete", stage6_unload_keeper },
    { "nested-unload", stage6_unload_declarer },
    { "nounload", stage6_unload_nobody },
    { "timeouts", stage6_time_out_shutdown },
    { "slots", stage6_take_slot },
    { "reload", stage6_reload_ticker },
    { "reloadexact", stage6_reload_ticker },
    { "reloadexactfail", stage6_reload_ticker },
    { "reloadspawns", stage6_reload_ticker },
    { "reloadlayout", stage6_reload_ticker },
    { "reloadfail", stage6_reload_ticker },
    { "sharegrow", stage6_grow_share },
    { "sharechain", stage6_grow_share },
    { "sharesame", stage6_grow_share },
    { "shareless", stage6_grow_share },
    { "sharebig", stage6_grow_share },
    { "repair", stage6_repair },
    { "repairload", stage6_repair_by_load },
    { "reloadmove", stage6_reload_ticker },
    { "reloaddrop", stage6_reload_ticker },
    { "maynot", stage6_reload_ticker },
    { "revive", stage6_revive },
    { "noreload", stage6_leave_reload_out },
    { "notreloadable", stage6_allow_not_reloadable },
    { "reloadnothing", stage6_reload_nothing },
    { "nreload", stage6_reload_native },
    { "nreloadfail", stage6_reload_native },
    { "nreloadfailunload", stage6_reload_native },
    { "nreloadfailstop", stage6_reload_native },
    { "ntimeout", stage6_state_native_timeout },
    { "config", stage6_configure },
    { "configkey", stage6_misname_setting },
    { "configkind", stage6_mistype_setting },
    { "configlong", stage6_overflow_text },
    { "configrange", stage6_overflow_integer },
    { "configwide", stage6_overflow_enum },
    { "configenum", stage6_misname_constant },
    { "confignone", stage6_state_no_config },
    { "configunset", stage6_leave_config_unset },
    { "configmissing", stage6_name_missing_config },
    { "configarena", stage6_state_small_arena },
    { "configtwice", stage6_configure_twice },
    { "configreload", stage6_reload_configured },
    { "configdrop", stage6_drop_configured },
    { "reseed", stage6_reseed },
    { "confignoroom", stage6_leave_no_room },
    { "configtable", stage6_configure_table },
    { "churn", stage6_churn },
    { "arenaback", stage6_use_arena },
    { "arenakept", stage6_use_arena },
    { "destroy", stage6_destroy_loaded }
};

/*
** main
**
** Runs the case the command line names, on a host with both backends and
** both phases, whose sink counts, and with a module's settings of every
** kind: a bitmask by its bits, an integer into a float field, and one of
** more than 32 bits.
*/
int main(int argc, char **argv)
{
    constexpr size_t count = sizeof(STAGE6_CASES) / sizeof(*STAGE6_CASES);
    stage6_t more = { .settings = {
        { .key = "vsync", .kind = BRIDGE_BOOL, .v = { .b = true } },
        { .key = "width", .kind = BRIDGE_INT, .v = { .i = 1280 } },
        { .key = "scale", .kind = BRIDGE_FLOAT, .v = { .f = 1.5 } },
        { .key = "mode", .kind = BRIDGE_STRING, .v = { .s = "ADAPTIVE" } },
        { .key = "title", .kind = BRIDGE_STRING, .v = { .s = "hello" } },
        { .key = "path", .kind = BRIDGE_STRING,
            .v = { .s = "some/long/path.cfg" } },
        { .key = "small", .kind = BRIDGE_INT, .v = { .i = 7 } },
        { .key = "flags", .kind = BRIDGE_INT, .v = { .i = 3 } },
        { .key = "ratio", .kind = BRIDGE_INT, .v = { .i = 3 } },
        { .key = "big", .kind = BRIDGE_INT, .v = { .i = 8'589'934'592ll } }
    } };
    host_t h;

    host_init(&h, argc, argv);
    h.sink = stage6_hear;
    h.ud = &more;
    return (host_run(&h, STAGE6_CASES, count));
}
