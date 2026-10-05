/* A host, as far as phase 5: order, observers, threads.
 *
 *     phase5 <case>
 *
 * Each case is one clause of docs/PLAN.md phase 5, checked by the records it emits. A record is
 * printed with the barrier count it was written under, since two of the claims are about which
 * barrier something happened at. */
#include "bridge.h"
#include "bridge-abi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define thread_id() ((uint32_t)GetCurrentThreadId())
#else
#include <pthread.h>
#define thread_id() ((uint32_t)(uintptr_t)pthread_self())
#endif

#define MS 1000000ll

/* the thread a native test module said it ran on, for the lines that say so */
static uint32_t threads_seen[64], seen, pins, pins_here;

static void sink(void *ud, const bridge_record *r)
{
    (void)ud;
    if (r->len == 3 && !memcmp(r->text, "pin", 3)) {
        pins ++;
        pins_here += r->arg[0] == thread_id();
        return;
    }
    if (r->len == 3 && !memcmp(r->text, "nap", 3)) {
        if (seen < 64) threads_seen[seen ++] = r->arg[0];
        return;
    }
    printf("record[%d] b%u %s/%s: %.*s %u %u\n", r->level, r->when >> 16,
        r->identity ? r->identity : "-", r->system ? r->system : "-", (int)r->len, r->text,
        r->arg[0], r->arg[1]);
}

static uint32_t distinct_threads(void)
{
    uint32_t i, k, n = 0;
    for (i = 0; i < seen; i ++) {
        for (k = 0; k < i && threads_seen[k] != threads_seen[i]; k ++) {}
        if (k == i) n ++;
    }
    return n;
}

static bridge_artifact native(const char *identity, const char *path)
{
    bridge_artifact a = {0};
    a.cfg.identity = identity;
    a.cfg.abi = BRIDGE_ABI;
    a.cfg.trusted = true;
    a.cfg.on_error = BRIDGE_SKIP;
    a.cfg.fuel = BRIDGE_NO_LIMIT;
    a.cfg.memory_limit = BRIDGE_NO_LIMIT;
    a.cfg.init_timeout = BRIDGE_NO_LIMIT;
    a.cfg.scratch = 64 << 10;
    a.cfg.log_ring = 64 * 24;
    a.cfg.spans = 16 << 10;
    a.backend = "native";
    a.path = path;
    return a;
}

static const char *const game[] = { "game:Position", "game:Marked" };

/* a wasm module of tests/modules, with the limits that backend can keep */
static bridge_artifact wasm(const char *identity, const char *file)
{
    bridge_artifact a = native(identity, NULL);
    char path[1024];
    FILE *f;
    void *data;
    snprintf(path, sizeof path, "%s/%s.wasm", MODULE_DIR, file);
    f = fopen(path, "rb");
    if (!f) { printf("cannot read %s\n", path); exit(2); }
    fseek(f, 0, SEEK_END);
    a.len = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(a.len);
    if (fread(data, 1, a.len, f) != a.len) { printf("cannot read %s\n", path); exit(2); }
    fclose(f);
    a.bytes = data;
    a.backend = "wasm";
    a.cfg.trusted = false;
    a.cfg.fuel = 10000000;
    a.cfg.memory_limit = 16 << 20;
    a.cfg.init_timeout = 2000 * MS;
    return a;
}

static void load(bridge *b, const bridge_artifact *a)
{
    printf("%s: %s\n", a->cfg.identity, bridge_load(b, a) ? "not loaded" : "loaded");
}

static void frames(bridge *b, int count)
{
    int i;
    for (i = 0; i < count; i ++) {
        bridge_phase(b, "update");
        bridge_phase(b, "present");
    }
}

static double phase_ms(bridge *b, const char *phase)
{
    struct timespec t0, t1;
    timespec_get(&t0, TIME_UTC);
    bridge_phase(b, phase);
    timespec_get(&t1, TIME_UTC);
    return (double)(t1.tv_sec - t0.tv_sec) * 1e3 + (double)(t1.tv_nsec - t0.tv_nsec) / 1e6;
}

/* bridge_phase from a thread that loaded nothing */
static bridge *elsewhere_bridge;
static int elsewhere_result;

#ifdef _WIN32
static DWORD WINAPI elsewhere(LPVOID arg)
{
    (void)arg;
    elsewhere_result = bridge_phase(elsewhere_bridge, "update");
    return 0;
}

static void from_another_thread(bridge *b)
{
    HANDLE t;
    elsewhere_bridge = b;
    t = CreateThread(NULL, 0, elsewhere, NULL, 0, NULL);
    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
}
#else
static void *elsewhere(void *arg)
{
    (void)arg;
    elsewhere_result = bridge_phase(elsewhere_bridge, "update");
    return NULL;
}

static void from_another_thread(bridge *b)
{
    pthread_t t;
    elsewhere_bridge = b;
    pthread_create(&t, NULL, elsewhere, NULL);
    pthread_join(t, NULL);
}
#endif

int main(int argc, char **argv)
{
    const char *what = argc > 1 ? argv[1] : "";
    bridge_backend backends[2] = {{ "wasm", BACKEND_PATH }, { "native", NATIVE_PATH }};
    const char *phases[2] = { "update", "present" };
    bridge_order pairs[2] = {{ "game:mover:move", "view:halver:halve" },
                             { "view:halver:halve", "game:mover:move" }};
    bridge_order seen_first = { "view:watcher:seen", "view:fixer:fix" };
    bridge_order fix_first = { "view:fixer:fix", "view:watcher:seen" };
    bridge_config cfg = { backends, 2, phases, 2, NULL, 0, 1, { 4096, 4096, 1 << 20, 8 } };
    bridge_artifact a;
    bridge *b;
    setvbuf(stdout, NULL, _IONBF, 0);

    if (!strcmp(what, "order")) { cfg.order = &pairs[0]; cfg.order_count = 1; }
    if (!strcmp(what, "reorder")) { cfg.order = &pairs[1]; cfg.order_count = 1; }
    if (!strcmp(what, "cycle")) { cfg.order = pairs; cfg.order_count = 2; }
    if (!strcmp(what, "obsorder")) { cfg.order = &seen_first; cfg.order_count = 1; }
    if (!strcmp(what, "obsreorder")) { cfg.order = &fix_first; cfg.order_count = 1; }
    if (!strcmp(what, "parallel")) cfg.threads = 2;
    if (!strcmp(what, "pinned") || !strcmp(what, "wrongthread")) cfg.threads = 4;
    if (!strcmp(what, "hop")) cfg.threads = 4;
    if (!strcmp(what, "nothreads")) cfg.threads = 0;

    b = bridge_create(&cfg, sink, NULL);
    if (!b) {
        printf("bridge_create refused\nalive\n");
        return 0;
    }

    if (!strcmp(what, "order") || !strcmp(what, "reorder") || !strcmp(what, "ambiguous")) {
        /* the mover adds 1 and the halver halves, in one phase: which first is the sum */
        a = wasm("game:mover@0.1.0", "mover");
        load(b, &a);
        a = wasm("view:halver@0.1.0", "halver");
        a.cfg.write = game;
        a.cfg.write_count = 1;
        load(b, &a);
        a = wasm("view:reader@0.1.0", "reader");
        a.cfg.read = game;
        a.cfg.read_count = 2;
        load(b, &a);
        frames(b, 1);
    } else if (!strcmp(what, "hop")) {
        /* wasm instances on four threads: a store is called from whichever thread took its
         * module's job, never from two at once. Fifty frames, and the last sum is exact. */
        a = wasm("game:mover@0.1.0", "mover");
        load(b, &a);
        a = wasm("view:reader@0.1.0", "reader");
        a.cfg.read = game;
        a.cfg.read_count = 2;
        load(b, &a);
        frames(b, 50);
    } else if (!strcmp(what, "ownorder")) {
        a = wasm("game:twostep@0.1.0", "twostep");
        load(b, &a);
        frames(b, 1);
    } else if (!strcmp(what, "observe")) {      /* a native attach, a wasm observer */
        a = native("game:setter@0.1.0", NSETTER_PATH);
        load(b, &a);
        a = wasm("view:watcher@0.1.0", "watcher");
        a.cfg.read = game;
        a.cfg.read_count = 2;
        load(b, &a);
        frames(b, 2);
    } else if (!strcmp(what, "obsconflict") || !strcmp(what, "obsorder")
               || !strcmp(what, "obsreorder")) {
        /* two observers of one change, in two modules, one of them writing what the other
         * reads: the barrier is a phase like any other to the conflict graph */
        a = native("game:setter@0.1.0", NSETTER_PATH);
        load(b, &a);
        a = wasm("view:watcher@0.1.0", "watcher");
        a.cfg.read = game;
        a.cfg.read_count = 2;
        load(b, &a);
        a = wasm("view:fixer@0.1.0", "fixer");
        a.cfg.write = &game[1];
        a.cfg.write_count = 1;
        load(b, &a);
        frames(b, 1);
    } else if (!strcmp(what, "events")) {       /* add and set, then set, then remove */
        a = wasm("game:events@0.1.0", "events");
        load(b, &a);
        frames(b, 3);
    } else if (!strcmp(what, "chain")) {
        a = wasm("game:chain@0.1.0", "chain");
        load(b, &a);
        frames(b, 3);
    } else if (!strcmp(what, "reserved")) {
        a = wasm("game:badobs@0.1.0", "badobs");
        load(b, &a);
    } else if (!strcmp(what, "parallel") || !strcmp(what, "serial")) {
        double ms;
        a = native("test:sleep1@0.1.0", NSLEEP1_PATH);
        load(b, &a);
        a = native("test:sleep2@0.1.0", NSLEEP2_PATH);
        load(b, &a);
        ms = phase_ms(b, "update");
        printf("two naps of 100 ms took %.0f ms on %u thread(s)\n", ms, distinct_threads());
        printf("%s\n", ms < 180.0 && distinct_threads() == 2 ? "they ran at once"
             : ms > 190.0 && distinct_threads() == 1 ? "they ran one after the other"
             : "neither clearly");
    } else if (!strcmp(what, "pinned")) {       /* three systems, pinned, among others */
        a = native("test:sleep1@0.1.0", NSLEEP1_PATH);
        load(b, &a);
        a = native("test:sleep2@0.1.0", NSLEEP2_PATH);
        load(b, &a);
        a = native("test:pinned@0.1.0", NPINNED_PATH);
        a.cfg.pinned = true;
        load(b, &a);
        frames(b, 3);
        /* the naps run beside it, on whichever threads: its own lines are the ones counted */
        printf("the others used %u thread(s)\n", distinct_threads());
        printf("pinned: %u of its %u calls were on the thread that loaded it\n", pins_here, pins);
    } else if (!strcmp(what, "wrongthread")) {
        a = native("test:pinned@0.1.0", NPINNED_PATH);
        a.cfg.pinned = true;
        load(b, &a);
        from_another_thread(b);
        printf("from another thread: %s\n", elsewhere_result ? "refused" : "ran");
    }

    bridge_destroy(b);
    printf("alive\n");
    return 0;
}
