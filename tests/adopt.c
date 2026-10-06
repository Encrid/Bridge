/* One block, adopted by the wasm backend, backing a memory in two stores at once.
 *
 * This is the question stage 2 opens with: Bridge-ASM700 allocates the shared memory and
 * every module imports it, each from a store of its own. wasmtime does not promise that
 * one block may back a memory twice, so it is tried here before anything is built on it. */
#include "backend.h"
#include "os.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constexpr uint32_t PAGE = 65'536;

static backend_api_t api;
static backend_t *be;
static backend_module_t *module;
static backend_memory_t *memory;

static void *read_file(const char *path, size_t *len)
{
    FILE *f = fopen(path, "rb");
    void *data;
    if (!f) { printf("FAIL: cannot read %s\n", path); exit(1); }
    fseek(f, 0, SEEK_END);
    *len = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(*len);
    if (fread(data, 1, *len, f) != *len) { printf("FAIL: cannot read %s\n", path); exit(1); }
    fclose(f);
    return data;
}

static backend_instance_t *instance()
{
    backend_err_t err = {};
    backend_memory_import_t imports[1] = {{ "bridge", "shared", memory }};
    backend_instance_t *i = api.instance_create(module, imports, 1, &err);
    if (!i) { printf("FAIL: instance_create: %s\n", err.text); exit(1); }
    return i;
}

/* 0 and the function's result, or -1 and why */
static int run(backend_instance_t *i, const char *name, int32_t *status, backend_err_t *err)
{
    backend_func_t *f = api.instance_get_func(i, name, err);
    if (!f) return -1;
    return api.call(f, status, err);
}

/* what the host answers a module that calls it: both arguments, added */
static int ping([[maybe_unused]] void *ud, const uint64_t *args, uint64_t *results)
{
    results[0] = args[0] + args[1];
    return 0;
}

typedef struct { const char *name; int rounds, failed; } job_t;

static void worker(void *arg)
{
    job_t *j = arg;
    backend_instance_t *i = instance();       /* a store of this thread's own */
    backend_err_t err = {};
    int32_t status;
    int r;
    for (r = 0; r < j->rounds; r ++) {
        if (run(i, j->name, &status, &err) || status) j->failed ++;
    }
    api.instance_destroy(i);
}

int main()
{
    backend_err_t err = {};
    uint64_t reservation, guard, granule;
    size_t len, k;
    int32_t status = -1;
    int bad = 0;

    setvbuf(stdout, nullptr, _IONBF, 0);       /* a crash must not swallow what was said */
    if (!backend_open(BACKEND_PATH, &api, &err)) { printf("FAIL: %s\n", err.text); return 1; }
    be = api.backend_create(nullptr, 0, &err);
    api.memory_get_shape(be, &reservation, &guard, &granule);
    printf("shape: reservation %llu MiB, guard %llu MiB\n",
        (unsigned long long)(reservation >> 20), (unsigned long long)(guard >> 20));

    /* the block is Bridge-ASM700's: allocated here, to the shape the backend stated */
    unsigned char *block = os_reserve((size_t)(reservation + guard), 2 * PAGE);
    if (!block) { printf("FAIL: cannot reserve the block\n"); return 1; }
    memory = api.memory_adopt(be, block, 2 * PAGE, &err);

    void *bytes = read_file(MODULE_DIR "/adopt.wasm", &len);
    module = api.module_compile(be, &(backend_source_t){ bytes, len, nullptr, 0 }, &err);
    if (!module) { printf("FAIL: module_compile: %s\n", err.text); return 1; }

    /* 1. a module writes, the host reads the same bytes */
    backend_instance_t *one = instance();
    if (run(one, "write", &status, &err) || status) { printf("FAIL: write: %s\n", err.text); return 1; }
    if (memcmp(block + 16, "\x0D\xF0\x0D\x60", 4)) { printf("FAIL: the host does not see the write\n"); return 1; }
    printf("one store: the host reads what the module wrote\n");

    /* 2. two stores on two threads, the same block, disjoint halves */
    job_t low = { "fill_low", 200, 0 }, high = { "fill_high", 200, 0 };
    os_thread_t *a = os_thread_start(worker, &low), *b = os_thread_start(worker, &high);
    os_thread_join(a);
    os_thread_join(b);
    for (k = 0; k < PAGE; k ++) bad += block[k] != 0xAA;
    for (k = PAGE; k < 2 * PAGE; k ++) bad += block[k] != 0xBB;
    if (low.failed || high.failed || bad) {
        printf("FAIL: two stores: %d and %d calls failed, %d bytes wrong\n", low.failed, high.failed, bad);
        return 1;
    }
    /* and a third store reads what the second one wrote */
    if (run(one, "sum_high", &status, &err) || status != 0xBB * (int32_t)PAGE) {
        printf("FAIL: a store does not see another store's writes (%d): %s\n", status, err.text);
        return 1;
    }
    printf("two stores, two threads: both halves intact, and each sees the other's\n");

    /* 3. past the block: a trap, and everything still works afterwards */
    if (run(one, "stray", &status, &err) == 0) {
        printf("FAIL: a load past the block returned %d instead of trapping\n", status);
        return 1;
    }
    printf("stray load: %s\n", err.text);
    if (run(one, "write", &status, &err) || status) { printf("FAIL: after the trap: %s\n", err.text); return 1; }
    printf("after the trap: the store still runs\n");

    /* 4. a function the host defines, with the signature it declared */
    {
        static const uint8_t params[2] = { BACKEND_I32, BACKEND_I64 }, results[1] = { BACKEND_I32 };
        backend_sig_t sig = { .params = params, .param_count = 2, .results = results,
            .result_count = 1 };
        void *host_bytes = read_file(MODULE_DIR "/host.wasm", &len);
        backend_module_t *host = api.module_compile(be, &(backend_source_t){ host_bytes, len, nullptr, 0 }, &err);
        backend_instance_t *hi = host ? api.instance_create(host, nullptr, 0, &err) : nullptr;
        if (!hi || api.define_func(hi, &(backend_entry_t){ "bridge", "ping", ping, nullptr, &sig }, &err)
            || run(hi, "init", &status, &err) || status != 16) {
            printf("FAIL: a host function: %d: %s\n", status, err.text);
            return 1;
        }
        printf("a host function: called with an i32 and an i64, answered %d\n", status);
        api.instance_destroy(hi);
        api.module_destroy(host);
    }

    /* 5. timed calls back to back. Each is over long before its second is up, so none may
     * be interrupted, and the backend must still be able to stop its clock afterwards: a
     * watchdog that counted wake-ups instead of looking would lose one here and wait forever. */
    {
        static const uint8_t params[2] = { BACKEND_I32, BACKEND_I64 }, results[1] = { BACKEND_I32 };
        backend_sig_t sig = { .params = params, .param_count = 2, .results = results,
            .result_count = 1 };
        void *host_bytes = read_file(MODULE_DIR "/host.wasm", &len);
        backend_module_t *timed = api.module_compile(be, &(backend_source_t){ host_bytes, len, nullptr, BACKEND_TIMEOUT }, &err);
        backend_instance_t *ti = timed ? api.instance_create(timed, nullptr, 0, &err) : nullptr;
        backend_func_t *f;
        int k;
        if (!ti || api.define_func(ti, &(backend_entry_t){ "bridge", "ping", ping, nullptr, &sig }, &err)
            || !(f = api.instance_get_func(ti, "init", &err))) {
            printf("FAIL: timed calls: %s\n", err.text);
            return 1;
        }
        for (k = 0; k < 20'000; k ++) {
            api.instance_set_timeout(ti, 1'000'000'000ull);
            if (api.call(f, &status, &err) || status != 16) {
                printf("FAIL: timed call %d: %d: %s\n", k, status, err.text);
                return 1;
            }
        }
        printf("20000 timed calls back to back: none interrupted\n");
        api.instance_destroy(ti);
        api.module_destroy(timed);
    }

    api.instance_destroy(one);
    api.module_destroy(module);
    api.memory_release(memory);
    api.backend_destroy(be);
    os_release(block, (size_t)(reservation + guard));
    return 0;
}
