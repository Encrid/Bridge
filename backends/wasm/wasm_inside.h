/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** backends/wasm/wasm_inside.h -- the wasm backend: the seventeen entries of
** bridge_backend.h, over wasmtime. What its files share.
**
** wasmtime.md, *Memory*
*/

#ifndef BRIDGE_BACKENDS_WASM_INSIDE_H
    #define BRIDGE_BACKENDS_WASM_INSIDE_H

    #include <wasm.h>
    #include <wasmtime.h>

    #include "bridge_backend.h"
    #include "os.h"

/*
** wasmtime enforces a 32-bit memory's bounds by faulting: the whole 4 GiB an
** index can reach is reserved, then a guard, and loads are emitted with no
** check. Stated here and to wasmtime both, so that what memory_get_shape
** answers is what the engine assumes. A wasm memory is a whole number of
** pages.
*/
constexpr uint64_t WASM_RESERVATION = (uint64_t)1 << 32;
constexpr uint64_t WASM_GUARD = (uint64_t)32 << 20;
constexpr uint32_t WASM_PAGE = 65'536;

/*
** A deadline no call reaches: every store needs one once epochs are on.
*/
constexpr uint64_t WASM_FAR = (uint64_t)1 << 62;

/*
** Fuel and epoch interruption are an engine's settings and put code in
** every loop, so there is an engine per combination a module can demand,
** made at its first use. A kind of engine is a mask of these.
*/
enum wasm_engine : uint8_t {
    WASM_METERED = 1,
    WASM_INTERRUPTIBLE = 2
};

constexpr int WASM_ENGINES = 4;

/*
** The backend.
**
**     wake          something below changed
**     lock          over the six below
**     active        which timed call is running: 0 for none
**     calls         how many there have been, to tell one from the next
**     until         when the running one is out of time, by os_get_now
**     ticking       and the engine it runs on
**     adopting      the adopted block the next memory wasmtime asks for is,
**                   or nullptr: wasmtime asks for every linear memory, and
**                   is not told which. Set around the instantiation that
**                   asks, with the lock held for as long as it is set
*/
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

/*
** What the watchdog reads of the backend, under its lock.
*/
typedef struct {
    uint64_t active;
    uint64_t until;
    wasm_engine_t *engine;
    int quit;
} wasm_armed_t;

/*
** An adopted block, and the module that holds it for each engine that has
** used it: built once, where it was built for every instance.
*/
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

/*
** An instance. `deadline` is in nanoseconds, for the next call only.
*/
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

/*
** A memory a module defines for itself: reserved whole, and committed as it
** grows.
*/
typedef struct {
    uint8_t *base;
    size_t size;
    size_t reserve;
} wasm_own_t;

/*
** A function Bridge offers a module: what answers it, and the kinds
** of what it takes and gives.
*/
typedef struct {
    backend_fn_t fn;
    void *ud;
    uint32_t params;
    uint32_t results;
    uint8_t param_kind[16];
    uint8_t result_kind[4];
} wasm_offered_t;

/*
** wasm_drop
**
** Deletes an error, if there is one: wasmtime_error_delete takes ownership
** of a real error, and must not be handed nullptr.
*/
void wasm_drop(wasmtime_error_t *error);

/*
** wasm_fail
**
** Writes why something failed: `what`, and the reason wasmtime gives in an
** error or a trap, which it deletes. `err` may be nullptr. Answers -1.
*/
int wasm_fail(backend_err_t *err, const char *what, wasmtime_error_t *error,
    wasm_trap_t *trap);

/*
** wasm_make_own
**
** Answers wasmtime with a memory a module defines for itself.
*/
wasmtime_error_t *wasm_make_own(size_t minimum, wasmtime_linear_memory_t *ret);

/*
** wasm_adopt_into
**
** Puts an adopted block into an instance's store, and answers it as an
** item a linker can define.
*/
wasmtime_error_t *wasm_adopt_into(backend_instance_t *i, backend_memory_t *mem,
    wasmtime_extern_t *item, wasm_trap_t **trap);

/*
** wasm_watch
**
** The watchdog's thread: one armed deadline at a time, and no tick period.
*/
void wasm_watch(void *arg);

/*
** wasm_get_engine
**
** Answers the engine of a kind, made at its first use.
*/
wasm_engine_t *wasm_get_engine(backend_t *b, enum wasm_engine kind);

/*
** wasm_make_offered
**
** Answers what wasmtime is handed with a function Bridge offers.
*/
wasm_offered_t *wasm_make_offered(const backend_entry_t *entry);

/*
** wasm_make_types
**
** Writes the types of wasmtime's that a signature's kinds are.
*/
void wasm_make_types(wasm_valtype_vec_t *vec, const enum backend_type *kinds,
    uint32_t count);

/*
** wasm_call_offered
**
** What wasmtime calls when a module calls a function Bridge offers.
*/
wasm_trap_t *wasm_call_offered(void *env, wasmtime_caller_t *caller,
    wasmtime_val_raw_t *raw, size_t count);

#endif
