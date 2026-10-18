/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_CASES_H
    #define TEST_CASES_H

    #include <stdio.h>
    #include <string.h>

    #include "host.h"

typedef struct {
    const char *what;
    const char *path;
    int64_t sets;
    const char *phases[12];
} test_kept_case_t;

void test_hear(void *ud, const bridge_record_t *r);

void test_set(host_t *h);

void test_load(host_t *h, const char *identity, const char *path,
    int64_t spawns);

void test_load_watcher(host_t *h);

void test_run_mover(host_t *h);
void test_run_cover(host_t *h);
void test_refuse_entry(host_t *h);
void test_enter_from_foreign_thread(host_t *h);
void test_observe(host_t *h);
void test_order_changes(host_t *h);
void test_state_untrusted(host_t *h);
void test_state_limit(host_t *h);
void test_state_no_path(host_t *h);
void test_name_no_library(host_t *h);
void test_hand_setting(host_t *h);
void test_read_kept(host_t *h);
void test_keep_bytes(host_t *h);
void test_load_no_init(host_t *h);

#endif
