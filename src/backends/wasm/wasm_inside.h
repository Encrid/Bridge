/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_BACKENDS_WASM_INSIDE_H
    #define BRIDGE_BACKENDS_WASM_INSIDE_H

    #include <wasm.h>
    #include <wasmtime.h>

    #include "bridge_backend.h"
    #include "os.h"

constexpr uint64_t WASM_RESERVATION = (uint64_t)1 << 32;
constexpr uint64_t WASM_GUARD = (uint64_t)32 << 20;
constexpr uint32_t WASM_PAGE = 65'536;

constexpr uint64_t WASM_FAR = (uint64_t)1 << 62;

enum wasm_engine : uint8_t {
    WASM_METERED = 1,
    WASM_INTERRUPTIBLE = 2
};

constexpr int WASM_ENGINES = 4;

struct backend {
    wasm_engine_t *engines[WASM_ENGINES];
    os_thread_t *watchdog;
    os_event_t *wake;
    os_mutex_t *lock;
    uint64_t active;
    uint64_t calls;
    uint64_t until;
    wasm_engine_t *ticking;
    backend_memory_t *adopting;
    int quit;
};

typedef struct {
    uint64_t active;
    uint64_t until;
    wasm_engine_t *engine;
    int quit;
} wasm_armed_t;

struct backend_memory {
    void *ptr;
    uint64_t len;
    wasmtime_module_t *holder[WASM_ENGINES];
};

struct backend_module {
    backend_t *b;
    enum wasm_engine kind;
    wasmtime_module_t *module;
    wasm_importtype_vec_t imports;
};

struct backend_func {
    backend_instance_t *instance;
    wasmtime_func_t func;
    backend_func_t *next;
};

struct backend_instance {
    backend_t *b;
    backend_module_t *m;
    wasm_engine_t *engine;
    wasmtime_store_t *store;
    wasmtime_context_t *ctx;
    wasmtime_linker_t *linker;
    wasmtime_instance_t instance;
    int instantiated;
    uint64_t deadline;
    backend_func_t *funcs;
};

typedef struct {
    uint8_t *base;
    size_t size;
    size_t reserve;
} wasm_own_t;

typedef struct {
    backend_fn_t fn;
    void *ud;
    uint32_t params;
    uint32_t results;
    uint8_t param_kind[16];
    uint8_t result_kind[4];
} wasm_offered_t;

void wasm_drop(wasmtime_error_t *error);

int wasm_fail(backend_err_t *err, const char *what, wasmtime_error_t *error,
    wasm_trap_t *trap);

wasmtime_error_t *wasm_make_own(size_t minimum, wasmtime_linear_memory_t *ret);

wasmtime_error_t *wasm_adopt_into(backend_instance_t *i, backend_memory_t *mem,
    wasmtime_extern_t *item, wasm_trap_t **trap);

void wasm_watch(void *arg);

wasm_engine_t *wasm_get_engine(backend_t *b, enum wasm_engine kind);

wasm_offered_t *wasm_make_offered(const backend_entry_t *entry);

void wasm_make_types(wasm_valtype_vec_t *vec, const enum backend_type *kinds,
    uint32_t count);

wasm_trap_t *wasm_call_offered(void *env, wasmtime_caller_t *caller,
    wasmtime_val_raw_t *raw, size_t count);

#endif
