/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage2/stage2.h -- a host, as far as stage 2: it creates a bridge,
** loads one module, and prints every record. Each case is one claim of
** stage 2 of the plan. build.py checks a refusal by the record it emits,
** not by a return code alone, and every case ends by printing "alive".
*/

#ifndef TEST_STAGE2_H
    #define TEST_STAGE2_H

    #include "host.h"

/*
** stage2_print
**
** The sink of this program: prints a record's level and its text.
*/
void stage2_print(void *ud, const bridge_record_t *r);

/*
** stage2_state
**
** States the wasm module of a name as `test:<name>@0.1.0`, with the small
** limits this stage loads everything under.
*/
void stage2_state(bridge_artifact_t *a, char *identity, const char *name);

/*
** stage2_load
**
** Loads the wasm module of a name, stated as stage2_state has it.
*/
void stage2_load(host_t *h, const char *name);

/*
** The cases. Each is handed the host as main configured it.
*/
void stage2_load_named(host_t *h);
void stage2_create_no_sink(host_t *h);
void stage2_stop(host_t *h);
void stage2_state_no_limit(host_t *h);
void stage2_state_other_abi(host_t *h);
void stage2_name_no_backend(host_t *h);
void stage2_open_no_library(host_t *h);
void stage2_state_other_host_abi(host_t *h);
void stage2_state_no_host_abi(host_t *h);
void stage2_leave_unset(host_t *h);
void stage2_leave_level_unset(host_t *h);
void stage2_leave_flag_unset(host_t *h);
void stage2_state_negative(host_t *h);
void stage2_state_other_none(host_t *h);

#endif
