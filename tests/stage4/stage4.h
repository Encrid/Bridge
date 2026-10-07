/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage4/stage4.h -- a host, as far as stage 4: two backends, and
** modules on both. Each case is one clause of stage 4 of the plan, checked
** by the records it emits.
*/

#ifndef TEST_STAGE4_H
    #define TEST_STAGE4_H

    #include "host.h"

/*
** stage4_state
**
** States a native module that mints up to 1,024 ids between two barriers.
*/
void stage4_state(bridge_artifact_t *a, const char *identity,
    const char *path);

/*
** stage4_state_reader
**
** States the reader of stage 3, on the wasm backend, with the limits that
** backend can keep and its grant. It only reads.
*/
void stage4_state_reader(bridge_artifact_t *a);

/*
** The cases. Each is handed the host as main configured it.
*/
void stage4_run_native(host_t *h);
void stage4_run_mixed(host_t *h);
void stage4_state_untrusted(host_t *h);
void stage4_state_limit(host_t *h);
void stage4_state_no_path(host_t *h);
void stage4_state_no_bytes(host_t *h);
void stage4_name_no_library(host_t *h);
void stage4_refuse_entry(host_t *h);
void stage4_leave_libraries(host_t *h);
void stage4_open_other_seam(host_t *h);
void stage4_state_unknown_setting(host_t *h);

#endif
