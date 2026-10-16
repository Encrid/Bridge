/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_HOST_H
    #define TEST_HOST_H

    #include <stddef.h>

    #include "bridge.h"
    #include "bridge_abi.h"

constexpr int64_t HOST_MS = 1'000'000;

constexpr int64_t HOST_TESTS_KEEPS = 1 << 4 | 1 << 6;

constexpr int64_t HOST_TESTS_KEEPS_ALL = 63;

constexpr int64_t HOST_TESTS_RESERVES = INT64_C(1) << 32;
constexpr int64_t HOST_TESTS_GRANULE = 65'536;

typedef struct host {
    const char *what;
    const char *arg;
    void *kept;
    bridge_backend_t backends[2];
    const char *phases[2];
    bridge_config_t cfg;
    bridge_sink_fn_t sink;
    void *ud;
    bridge_t *b;
} host_t;

typedef struct {
    const char *name;
    void (*run)(host_t *h);
    const char *arg;
    void (*set)(host_t *h);
} host_case_t;

void host_init(host_t *h, int argc, char **argv);

int host_run(host_t *h, const host_case_t *cases, size_t count);

[[nodiscard]] int host_open(host_t *h);

void host_close(host_t *h);

void host_open_only(host_t *h);

void host_print_record(void *ud, const bridge_record_t *r);

void host_load(host_t *h, const bridge_artifact_t *a);
void host_reload(host_t *h, const bridge_artifact_t *a);
void host_unload(host_t *h, const char *identity);

void host_run_frames(host_t *h, int count);

void host_run_frames_unsaid(bridge_t *b, int count);

typedef struct {
    void (*run)(void *arg);
    void *arg;
    void *handle;
} host_thread_t;

uint32_t host_get_thread_id(void);

void host_thread_start(host_thread_t *t);
void host_thread_join(host_thread_t *t);

bool host_is_library_loaded(const char *path);

void host_name_module(bridge_artifact_t *a, const char *file);
void host_state_built(bridge_artifact_t *a, const char *identity,
    const char *name);
void host_state_plain(bridge_artifact_t *a, const char *identity,
    const char *path);

void host_state_guarded(bridge_artifact_t *a, const char *identity,
    const char *file);

#endif
