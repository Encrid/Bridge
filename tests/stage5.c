/* A host, as far as stage 5: order, observers, threads.
 *
 *     stage5 <case>
 *
 * Each case is one clause of docs/PLAN.md stage 5, checked by the records it emits. A record is
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

constexpr int64_t MS = 1'000'000;

/* the thread a native test module said it ran on, for the lines that say so */
static uint32_t threads_seen[64], began_seen[64], ended_seen[64], seen, ended, pins, pins_here;

/* two bridges, one sink: each says which it is, and its last sum is kept */
static uint32_t last_sum[2];

/* the ids a run's modules were given, in the order the records arrived */
static uint32_t minted[3][256], minted_count[3];
static int minting = -1;

static void sink(void *ud, const bridge_record *r)
{
    if (ud) {
        if (r->len == 3 && !memcmp(r->text, "sum", 3)) last_sum[*(int *)ud] = r->arg[0];
        return;
    }
    if (minting >= 0 && r->len == 3 && !memcmp(r->text, "ids", 3)) {
        uint32_t *n = &minted_count[minting];
        if (*n + 2 <= 256) { minted[minting][(*n) ++] = r->arg[0]; minted[minting][(*n) ++] = r->arg[1]; }
        return;
    }
    if (r->len == 3 && !memcmp(r->text, "pin", 3)) {
        pins ++;
        pins_here += r->arg[0] == thread_id();
        return;
    }
    if (r->len == 3 && !memcmp(r->text, "nap", 3)) {
        if (seen < 64) {
            began_seen[seen] = r->arg[1];
            threads_seen[seen ++] = r->arg[0];
        }
        return;
    }
    if (r->len == 3 && !memcmp(r->text, "end", 3)) {
        if (ended < 64) ended_seen[ended ++] = r->arg[1];
        return;
    }
    printf("record[%d] b%u %s/%s: %.*s %u %u\n", r->level, r->when >> 16,
        r->identity ? r->identity : "-", r->system ? r->system : "-", (int)r->len, r->text,
        r->arg[0], r->arg[1]);
}

static uint32_t distinct_threads()
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
    bridge_artifact a = {};
    a.cfg.identity = identity;
    a.cfg.abi = BRIDGE_ABI;
    a.cfg.trusted = true;
    a.cfg.on_error = BRIDGE_SKIP;
    a.cfg.max_system_failures = 3;
    a.cfg.fuel = BRIDGE_NO_LIMIT;
    a.cfg.memory_limit = BRIDGE_NO_LIMIT;
    a.cfg.init_timeout = BRIDGE_NO_LIMIT;
    a.cfg.reload_timeout = BRIDGE_NO_LIMIT;
    a.cfg.shutdown_timeout = BRIDGE_NO_LIMIT;
    a.cfg.scratch = 64 << 10;
    a.cfg.log_ring = 64 * 24;
    a.cfg.spans = 16 << 10;
    a.cfg.log_level = BRIDGE_INFO;
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.persistent = BRIDGE_NONE;
    a.cfg.changes = 1 << 20;
    a.cfg.config = BRIDGE_NO_CONFIG;
    a.backend = "native";
    a.path = path;
    return a;
}

static const char *const game[] = { "game:Position", "game:Marked" };

/* a wasm module of tests/modules, with the limits that backend can keep */
static bridge_artifact wasm(const char *identity, const char *file)
{
    bridge_artifact a = native(identity, nullptr);
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
    a.cfg.fuel = 10'000'000;
    a.cfg.memory_limit = 16 << 20;
    a.cfg.init_timeout = 2000 * MS;
    a.cfg.reload_timeout = 2000 * MS;
    a.cfg.shutdown_timeout = 2000 * MS;
    /* what each test module mints between two barriers: most mint nothing, and say so */
    a.cfg.spawns = !strcmp(file, "mover") ? 1024
                 : !strcmp(file, "twostep") || !strcmp(file, "chain") || !strcmp(file, "events")
                   || !strcmp(file, "nested") || !strcmp(file, "nested2") ? 8 : BRIDGE_NONE;
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
        (void)bridge_phase(b, "update");
        (void)bridge_phase(b, "present");
    }
}

static double phase_ms(bridge *b, const char *phase)
{
    struct timespec t0, t1;
    timespec_get(&t0, TIME_UTC);
    (void)bridge_phase(b, phase);
    timespec_get(&t1, TIME_UTC);
    return (double)(t1.tv_sec - t0.tv_sec) * 1e3 + (double)(t1.tv_nsec - t0.tv_nsec) / 1e6;
}

/* bridge_phase from a thread that loaded nothing: once, or for many frames */
static bridge *elsewhere_bridge;
static int elsewhere_result, elsewhere_frames;

static void elsewhere_work()
{
    int i;
    if (elsewhere_frames < 0) elsewhere_result = bridge_destroy(elsewhere_bridge);
    else if (!elsewhere_frames) elsewhere_result = bridge_phase(elsewhere_bridge, "update");
    for (i = 0; i < elsewhere_frames; i ++) {
        (void)bridge_phase(elsewhere_bridge, "update");
        (void)bridge_phase(elsewhere_bridge, "present");
    }
}

#ifdef _WIN32
static DWORD WINAPI elsewhere([[maybe_unused]] LPVOID arg)
{
    elsewhere_work();
    return 0;
}

static HANDLE elsewhere_thread;

static void start_elsewhere(bridge *b, int count)
{
    elsewhere_bridge = b;
    elsewhere_frames = count;
    elsewhere_thread = CreateThread(nullptr, 0, elsewhere, nullptr, 0, nullptr);
}

static void join_elsewhere()
{
    WaitForSingleObject(elsewhere_thread, INFINITE);
    CloseHandle(elsewhere_thread);
}
#else
static void *elsewhere([[maybe_unused]] void *arg)
{
    elsewhere_work();
    return nullptr;
}

static pthread_t elsewhere_thread;

static void start_elsewhere(bridge *b, int count)
{
    elsewhere_bridge = b;
    elsewhere_frames = count;
    pthread_create(&elsewhere_thread, nullptr, elsewhere, nullptr);
}

static void join_elsewhere()
{
    pthread_join(elsewhere_thread, nullptr);
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
    bridge_order five_first = { "game:stamp5:stamp", "game:stamp9:stamp" };
    bridge_config cfg = { backends, 2, phases, 2, nullptr, 0, 1, 
        { .entities = 4096, .tables = 64, .types = 64, .systems = 128, .terms = 8, .modules = 8,
          .persistent = 4096, .columns = 1 << 20 } };
    bridge_artifact a;
    bridge *b;
    setvbuf(stdout, nullptr, _IONBF, 0);

    if (!strcmp(what, "order")) { cfg.order = &pairs[0]; cfg.order_count = 1; }
    if (!strcmp(what, "reorder")) { cfg.order = &pairs[1]; cfg.order_count = 1; }
    if (!strcmp(what, "cycle")) { cfg.order = pairs; cfg.order_count = 2; }
    if (!strcmp(what, "obsorder")) { cfg.order = &seen_first; cfg.order_count = 1; }
    if (!strcmp(what, "obsreorder")) { cfg.order = &fix_first; cfg.order_count = 1; }
    if (!strcmp(what, "changeorder") || !strcmp(what, "changeloaded") || !strcmp(what, "bothtold")) {
        cfg.order = &five_first;
        cfg.order_count = 1;
    }
    /* one table, which the entities with a position take: one with a mark as well has none */
    if (!strcmp(what, "bothtold")) cfg.budget.tables = 1;
    if (!strcmp(what, "parallel")) cfg.threads = 2;
    if (!strcmp(what, "pinned") || !strcmp(what, "wrongthread")) cfg.threads = 4;
    if (!strcmp(what, "hop")) cfg.threads = 4;
    if (!strcmp(what, "nothreads")) cfg.threads = 0;

    if (!strcmp(what, "minted")) {
        /* Two modules spawn in one level. On one thread, then twice on four: the ids each
         * is given are the same all three times, because they were set aside for it at the
         * barrier and not taken in whatever order the threads arrived. */
        static const uint32_t threads[3] = { 1, 4, 4 };
        int run;
        for (run = 0; run < 3; run ++) {
            int f;
            cfg.threads = threads[run];
            b = bridge_create(&cfg, sink, nullptr);
            if (!b) { printf("bridge_create refused\n"); return 0; }
            minting = run;
            a = wasm("test:spawn1@0.1.0", "spawner");
            a.cfg.spawns = 16;
            (void)bridge_load(b, &a);
            a = wasm("test:spawn2@0.1.0", "spawner");
            a.cfg.spawns = 16;
            (void)bridge_load(b, &a);
            for (f = 0; f < 20; f ++) (void)bridge_phase(b, "update");
            (void)bridge_destroy(b);
        }
        printf("minted %u ids a run; on four threads %s, and again %s\n", minted_count[0],
            minted_count[0] == minted_count[1]
                && !memcmp(minted[0], minted[1], sizeof minted[0]) ? "the same" : "DIFFERENT",
            minted_count[0] == minted_count[2]
                && !memcmp(minted[0], minted[2], sizeof minted[0]) ? "the same" : "DIFFERENT");
        printf("alive\n");
        return 0;
    }

    if (!strcmp(what, "two")) {
        /* two bridges in one process, each with its own storage, running at once on two
         * threads. The same modules in both, a different number of frames in each: the
         * sums are exact, and each is its own. */
        static int which[2] = { 0, 1 };
        bridge *both[2];
        int k;
        cfg.threads = 2;
        for (k = 0; k < 2; k ++) {
            both[k] = bridge_create(&cfg, sink, &which[k]);
            if (!both[k]) { printf("bridge %d refused\n", k); return 0; }
            a = wasm("game:mover@0.1.0", "mover");
            (void)bridge_load(both[k], &a);
            a = wasm("view:reader@0.1.0", "reader");
            a.cfg.read = game;
            a.cfg.read_count = 2;
            (void)bridge_load(both[k], &a);
        }
        start_elsewhere(both[1], 30);
        frames(both[0], 50);
        join_elsewhere();
        printf("two bridges: sums %u and %u\n", last_sum[0], last_sum[1]);
        (void)bridge_destroy(both[0]);
        (void)bridge_destroy(both[1]);
        printf("alive\n");
        return 0;
    }

    b = bridge_create(&cfg, sink, nullptr);
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
        a.cfg.spawns = 8;
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
        a.cfg.spawns = 8;
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
    } else if (!strcmp(what, "reapread") || !strcmp(what, "reapwrite")) {
        /* a module of another namespace destroys what it finds: with a read grant it has no
         * stake in the entity, which stays; with a write grant it has, and the entity goes */
        a = native("game:stamp5@0.1.0", NSTAMP5_PATH);
        a.cfg.spawns = 8;
        load(b, &a);
        a = native("view:reaper@0.1.0", NREAPER_PATH);
        a.cfg.spawns = BRIDGE_NONE;
        if (!strcmp(what, "reapread")) { a.cfg.read = game; a.cfg.read_count = 1; }
        else { a.cfg.write = game; a.cfg.write_count = 1; }
        load(b, &a);
        frames(b, 2);
    } else if (!strcmp(what, "fewchanges") || !strcmp(what, "nochanges")) {
        /* its init attaches once, which is 32 bytes of queue: stated as 16, and as none */
        a = native("game:stamp5@0.1.0", NSTAMP5_PATH);
        a.cfg.spawns = 8;
        a.cfg.changes = !strcmp(what, "fewchanges") ? 16 : BRIDGE_NONE;
        load(b, &a);
    } else if (!strcmp(what, "unstated")) {
        /* a system attaches what it did not state among its changes: that invocation fails */
        a = native("game:stamp@0.1.0", NSTAMPUNSTATED_PATH);
        a.cfg.spawns = 8;
        load(b, &a);
        a = wasm("view:watcher@0.1.0", "watcher");
        a.cfg.read = game;
        a.cfg.read_count = 2;
        load(b, &a);
        frames(b, 2);
    } else if (!strcmp(what, "changeconflict") || !strcmp(what, "changeorder")
               || !strcmp(what, "changeloaded") || !strcmp(what, "bothtold")) {
        /* two systems of one phase, in two modules, that attach one component to the same
         * entities. Unordered, the second does not load. Ordered, the one stated to run last
         * is the one whose value stays, whichever of them was loaded first */
        bool nine_first = !strcmp(what, "changeloaded");
        a = native(nine_first ? "game:stamp9@0.1.0" : "game:stamp5@0.1.0",
            nine_first ? NSTAMP9_PATH : NSTAMP5_PATH);
        a.cfg.spawns = 8;
        load(b, &a);
        a = native(nine_first ? "game:stamp5@0.1.0" : "game:stamp9@0.1.0",
            nine_first ? NSTAMP5_PATH : NSTAMP9_PATH);
        a.cfg.spawns = 8;
        load(b, &a);
        a = wasm("view:watcher@0.1.0", "watcher");
        a.cfg.read = game;
        a.cfg.read_count = 2;
        load(b, &a);
        frames(b, 2);
    } else if (!strcmp(what, "nested") || !strcmp(what, "nested2")) {
        a = wasm("game:nested@0.1.0", what);
        load(b, &a);
        frames(b, 1);
    } else if (!strcmp(what, "events")) {       /* enter and attach, then attach, then leave */
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
        /* At once is one beginning before the other ended, and nothing about how long either
         * took: a sleep of a tenth of a second returns after two on some systems. The clock is
         * in milliseconds, so two that follow each other may share one: not an overlap. */
        bool both = seen == 2 && ended == 2;
        bool overlap = both && began_seen[0] < ended_seen[1] && began_seen[1] < ended_seen[0];
        printf("two naps took %.0f ms on %u thread(s)\n", ms, distinct_threads());
        printf("%s\n", both && overlap && distinct_threads() == 2 ? "they ran at once"
             : both && !overlap && distinct_threads() == 1 ? "they ran one after the other"
             : "neither clearly");
        if (both) {
            printf("one from %u to %u ms, the other from %u to %u\n", 0, ended_seen[0] - began_seen[0],
                began_seen[1] - began_seen[0], ended_seen[1] - began_seen[0]);
        }
    } else if (!strcmp(what, "foreign")) {      /* an entry reached from a thread nobody called on */
        a = native("test:foreign@0.1.0", NFOREIGN_PATH);
        a.cfg.spawns = 8;
        load(b, &a);
        frames(b, 1);
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
        start_elsewhere(b, 0);
        join_elsewhere();
        printf("from another thread: %s\n", elsewhere_result ? "refused" : "ran");
        /* and a destroy from there is refused too, leaving the bridge to be destroyed here */
        start_elsewhere(b, -1);
        join_elsewhere();
        printf("destroy from another thread: %s\n", elsewhere_result ? "refused" : "done");
        printf("destroy from the thread that loaded it: %s\n", bridge_destroy(b) ? "refused" : "done");
        printf("alive\n");
        return 0;
    }

    (void)bridge_destroy(b);
    printf("alive\n");
    return 0;
}
