/* Storage: what it does, said out loud, and what it costs.
 *
 *     storage changes     every structural change a module can ask for, one at a time, and what
 *                         a system and an observer see after each
 *     storage tables      ten thousand tables
 *     storage stress      four modules at once on four threads, eight million changes, each
 *                         keeping its own account of what should be there and checking it
 *     storage stress1     the same on one thread, which must say the same, word for word:
 *                         schedule.md, *What a pass depends on*
 *     storage notables    [budget] tables used up: the entity is left as it was, and it is a
 *     storage nocolumns   failure of the module that asked. The same for [budget] columns
 *     storage scribble    a module writes over what storage publishes in the shared block
 *     storage strip       a component stripped from every entity, a table at a time: into the
 *     storage stripfull   table of what is left, or where it stands. And with no room to merge
 *     storage measure     nanoseconds for each kind of change, at a barrier
 *     storage weigh       what the one storage layout there is costs where it is weakest: a
 *                         change to an entity that carries a lot, and a system whose rows are
 *                         spread over many tables. docs/plan/stage10.md, *The repair*
 *     storage baseline    what it costs at the work no storage layout exists for yet: a
 *                         component switched off by a field, a value reached by id, one
 *                         value for everything. docs/plan/stage11.md, *The baseline*
 *
 * The first two are a transcript and decide nothing themselves: build.py holds what it must
 * say. It is the same whatever stores the rows, which is the point - docs/plan/stage10.md.
 * The third is a measurement, and passes by running. From inside, for the barrier alone. */
#include "inside.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* What the process holds, in KB: the block's touched pages and everything outside it. Asked of
 * the operating system, so that it is the same question whatever keeps the rows. The figures of
 * docs/plan/stage10.md are taken on Windows, and elsewhere this says nothing. */
#ifdef _WIN32
typedef struct {
    uint32_t cb, faults;
    size_t peak, now, paged_peak, paged, nonpaged_peak, nonpaged, pagefile, pagefile_peak;
} held_counters_t;
[[gnu::dllimport]] void *GetCurrentProcess();
[[gnu::dllimport]] int K32GetProcessMemoryInfo(void *process, held_counters_t *counters, uint32_t size);

static void held(const bridge_t *b)
{
    held_counters_t c = { .cb = sizeof c };
    if (!K32GetProcessMemoryInfo(GetCurrentProcess(), &c, sizeof c)) return;
    printf("columns in use %u KB, and the process holds %zu KB\n", b->column_space.used >> 10, c.now >> 10);
}
#else
static void held(const bridge_t *b) { printf("columns in use %u KB\n", b->column_space.used >> 10); }
#endif

static void sink([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    printf("  %s: %.*s %u %u\n", r->system ? r->system : "-", (int)r->len, r->text, r->arg[0], r->arg[1]);
}

static void quiet([[maybe_unused]] void *ud, [[maybe_unused]] const bridge_record_t *r) { }

/* how many records said that memory ran out */
static uint32_t said_memory;
[[maybe_unused]] static void listening([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    for (uint32_t i = 0; i + 13 <= r->len; i ++) said_memory += !memcmp(r->text + i, "out of memory", 13);
}

/* what a system said it read: kept, so that a timing is of a loop that ran */
static uint32_t said[2];
static void counted([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    if (r->system) said[0] = r->arg[0], said[1] = r->arg[1];
    else if (r->level <= BRIDGE_WARN) printf("  -: %.*s\n", (int)r->len, r->text);
}

static bridge_artifact_t store_module(const char *path, int64_t spawns, uint32_t spans,
                                    int64_t changes)
{
    bridge_artifact_t a = {};
    a.cfg.identity = "st:store@0.1.0";
    a.cfg.scratch = 64 << 10;
    a.cfg.abi = BRIDGE_ABI;
    a.cfg.trusted = BRIDGE_TRUE;
    a.cfg.reloadable = BRIDGE_FALSE;
    a.cfg.hot_reload = BRIDGE_FALSE;
    a.cfg.pinned = BRIDGE_FALSE;
    a.cfg.on_error = BRIDGE_FAIL;
    a.cfg.max_system_failures = 1;
    a.cfg.work_limit = BRIDGE_NO_LIMIT;
    a.cfg.memory_limit = BRIDGE_NO_LIMIT;
    a.cfg.init_timeout = BRIDGE_NO_LIMIT;
    a.cfg.reload_timeout = BRIDGE_NO_LIMIT;
    a.cfg.shutdown_timeout = BRIDGE_NO_LIMIT;
    a.cfg.log_ring = 256 * 24;
    a.cfg.spans = spans;
    a.cfg.log_level = BRIDGE_INFO;
    a.cfg.spawns = spawns;
    a.cfg.persistent = BRIDGE_NONE;
    a.cfg.changes = changes;
    a.cfg.loss_count = BRIDGE_NONE;
    a.cfg.config = BRIDGE_NO_CONFIG;
    a.backend = "native";
    a.path = path;
    return a;
}

static const char *const phases[] = {
    "spawn", "attach_a", "attach_b", "detach_b", "destroy", "attach_b_half", "detach_b_half",
    "detach_b_absent", "value_a", "flicker", "destroy_half", "respawn_half", "touch_dead",
    "attach_a_half", "recycled", "combine", "look", "never", "work", "check", "attach_both",
    "scribble", "find", "value_b", "look_b", "walk", "flag", "near", "far", "one"
};
constexpr uint32_t PHASES = sizeof phases / sizeof *phases;

/* one phase and its barrier, named in the transcript */
/* budgets.md, *Nothing is allocated in a phase*. Built with COUNTED, the library's own calls
 * to the allocator are these, and a phase with its barrier is to make none. */
#ifdef COUNTED
static size_t allocations;
/* failure.md, *Out of memory*: and one of them can be made to answer that there is none.
 * `asked` counts the calls that ask for memory; the one numbered `fail_at` is answered null. */
static size_t asked, fail_at;
static bool failed;
static bool none_left() { asked ++; allocations ++; return asked == fail_at ? (failed = true) : false; }
void *counted_malloc(size_t n) { return none_left() ? nullptr : malloc(n); }
void *counted_calloc(size_t n, size_t each) { return none_left() ? nullptr : calloc(n, each); }
void *counted_realloc(void *p, size_t n) { return none_left() ? nullptr : realloc(p, n); }
void counted_free(void *p) { allocations ++; free(p); }
#else
constexpr size_t allocations = 0;
#endif

static void run(bridge_t *b, const char *phase)
{
    size_t before = allocations;
    printf("%s\n", phase);
    if (bridge_run_phase(b, phase)) printf("  REFUSED\n");
    if (allocations != before) {
        printf("  FAIL: the allocator was called %u times in a phase\n", (unsigned)(allocations - before));
    }
}

/* The systems of one phase, then the barrier, each timed: a change is the call that asks for
 * it and the barrier that applies it, and a library that applies a change at the call has one
 * figure where this has two. `called` is the first, of the phase last run. */
static uint64_t called;

static uint64_t barrier(bridge_t *b, const char *phase)
{
    uint32_t i;
    uint64_t t;
    for (i = 0; i < PHASES && strcmp(phases[i], phase); i ++) { }
    t = os_get_now();
    schedule_run(b, i);
    called = os_get_now() - t;
    t = os_get_now();
    block_barrier(b);
    return os_get_now() - t;
}

/* What unload does to a component with the policy `strip`, asked of storage directly so that the
 * module is still there to look: lifetime.md, *What unload leaves behind*. */
static void drop(bridge_t *b, const char *name, enum bridge_on_unload policy)
{
    static const char *const said[] = { "", "strip", "sweep", "delete" };
    printf("%s %s\n", said[policy], name);
    for (uint32_t i = 0; i < b->type_count; i ++) {
        if (!strcmp(b->types[i]->name, name)) storage_drop(b->storage, b->types[i]->type, policy);
    }
    /* a barrier, for what was heard and for what a module reads */
    run(b, "never");
}

static void strip(bridge_t *b, const char *name) { drop(b, name, BRIDGE_STRIP); }

/* the systems of one phase, timed, and then its barrier */
static uint64_t ran(bridge_t *b, const char *phase)
{
    uint32_t i;
    uint64_t t;
    for (i = 0; i < PHASES && strcmp(phases[i], phase); i ++) { }
    t = os_get_now();
    schedule_run(b, i);
    t = os_get_now() - t;
    block_barrier(b);
    return t;
}

/* lifetime.md, *When a component is moved*: a module that keeps st:X one way, reloaded as a
 * build that keeps it another. What storage has of it after is said from in here, whatever
 * became of the module. */
static const char *const kept_as[] = { "", "a table", "a sparse set", "a toggle", "direct-indexed", "a singleton" };

static void moved_case(bridge_config_t cfg, uint32_t from, uint32_t to, const char *prepare,
                       const char *const *losses, int64_t loss_count)
{
    static const char *const built[] = { "", NMOVE1_PATH, NMOVE2_PATH, NMOVE3_PATH, NMOVE4_PATH, NMOVE5_PATH };
    bridge_t *b = bridge_create(&cfg, sink, nullptr);
    bridge_artifact_t a = store_module(built[from], 8, 1 << 20, 4096);
    printf("%s to %s%s%s\n", kept_as[from], kept_as[to], *prepare ? ", after " : "", prepare);
    a.cfg.reloadable = a.cfg.hot_reload = BRIDGE_TRUE;
    if (!b || bridge_load(b, &a)) { printf("FAIL: not loaded\n"); exit(1); }
    (void)bridge_run_phase(b, "spawn");
    if (*prepare) (void)bridge_run_phase(b, prepare);
    (void)bridge_run_phase(b, "look");
    a.path = built[to];
    a.cfg.losses = losses;
    a.cfg.loss_count = loss_count;
    printf("  reload: %s\n", bridge_reload(b, &a) ? "refused" : "done");
    (void)bridge_run_phase(b, "look");
    for (uint32_t i = 0; i < b->type_count; i ++) {
        const declared_type_t *c = b->types[i];
        if (strcmp(c->name, "st:X")) continue;
        printf("  storage: st:X is %s, carried by %u\n", kept_as[c->storage], storage_count_carriers(b->storage, c->type));
    }
    (void)bridge_destroy(b);
}

/* lifetime.md, *When a component layout changed*: a module that states st:X one way, reloaded
 * as a build that states it another. */
static void relaid_case(bridge_config_t cfg, const char *from, const char *to, const char *what,
                        const char *const *losses, int64_t loss_count)
{
    bridge_t *b = bridge_create(&cfg, sink, nullptr);
    bridge_artifact_t a = store_module(from, 8, 1 << 20, 4096);
    printf("%s\n", what);
    a.cfg.reloadable = a.cfg.hot_reload = BRIDGE_TRUE;
    if (!b || bridge_load(b, &a)) { printf("FAIL: not loaded\n"); exit(1); }
    (void)bridge_run_phase(b, "spawn");
    (void)bridge_run_phase(b, "look");
    a.path = to;
    a.cfg.losses = losses;
    a.cfg.loss_count = loss_count;
    printf("  reload: %s\n", bridge_reload(b, &a) ? "refused" : "done");
    (void)bridge_run_phase(b, "look");
    (void)bridge_destroy(b);
}

/* Four of nstress, each in its own namespace, a hundred rounds of work and check. */
constexpr uint32_t STRESSED = 4, ROUNDS = 100;

static int stress_load(bridge_t *b)
{
    static const char *const identities[STRESSED] = {
        "s1:stress@0.1.0", "s2:stress@0.1.0", "s3:stress@0.1.0", "s4:stress@0.1.0"
    };
    static const char *const paths[STRESSED] = {
        NSTRESS1_PATH, NSTRESS2_PATH, NSTRESS3_PATH, NSTRESS4_PATH
    };
    for (uint32_t i = 0; i < STRESSED; i ++) {
        bridge_artifact_t a = store_module(paths[i], 20'000, 2 << 20, 16 << 20);
        a.cfg.identity = identities[i];
        a.cfg.scratch = 1 << 20;
        if (bridge_load(b, &a)) return 1;
    }
    return 0;
}

static void stress_run(bridge_t *b)
{
    for (uint32_t round = 0; round < ROUNDS; round ++) {
        if (bridge_run_phase(b, "work") || bridge_run_phase(b, "check")) printf("  REFUSED\n");
    }
}

int main(int argc, char **argv)
{
    const char *what = argc > 1 ? argv[1] : "";
    bridge_backend_t backends[1] = {{ "native", NATIVE_PATH }};
    /* the wasm backend beside it, which asks for address space around a block it maps */
    bridge_backend_t both[2] = {{ "wasm", BACKEND_PATH }, { "native", NATIVE_PATH }};
    bridge_config_t cfg = { .abi = BRIDGE_HOST_ABI, .backends = backends, .backend_count = 1, .phases = phases,
        .phase_count = PHASES, .threads = 1, .budget =
        { .entities = 400'000, .tables = 32'768, .types = 64, .systems = 64, .terms = 4, .modules = 4,
          .persistent = 4096, .columns = 64 << 20,
          .sparse_sets = BRIDGE_NONE, .direct_indexed = BRIDGE_NONE } };
    bridge_t *b;

    /* the stress run is the one thing here that is run on more than one thread */
    if (!strcmp(what, "stress")) cfg.threads = 4;
    if (!strcmp(what, "stress1")) what = "stress";      /* the same, on the caller's thread alone */
    if (!strcmp(what, "shape")) { cfg.backends = both; cfg.backend_count = 2; }
    if (!strcmp(what, "notables")) cfg.budget.tables = 1;
    if (!strcmp(what, "nocolumns")) cfg.budget.columns = 64;
    if (!strcmp(what, "stripfull")) cfg.budget.columns = 160;
    if (!strcmp(what, "togglefull")) cfg.budget.columns = 112;
    /* storage.md, *A sparse set*. `sparsenone` is the one that states there are none */
    if (!strncmp(what, "sparse", 6) && strcmp(what, "sparsenone")) cfg.budget.sparse_sets = 1;
    if (!strcmp(what, "weigh")) cfg.budget.sparse_sets = cfg.budget.direct_indexed = 1;
    /* storage.md, *Direct-indexed*. `directnone` is the one that states there are none */
    if (!strncmp(what, "direct", 6) && strcmp(what, "directnone")) cfg.budget.direct_indexed = 1;
    if (!strncmp(what, "tag", 3)) cfg.budget.sparse_sets = cfg.budget.direct_indexed = 1;
    if (!strcmp(what, "directfull")) cfg.budget.columns = 1024;
    if (!strcmp(what, "sparsefull")) cfg.budget.columns = argc > 2 ? (uint64_t)atoi(argv[2]) : 112;
    b = bridge_create(&cfg, sink, nullptr);
    bridge_artifact_t a;

    setvbuf(stdout, nullptr, _IONBF, 0);
    if (!b) { printf("FAIL: bridge_create\n"); return 1; }

#ifdef COUNTED
    if (!strcmp(what, "nomemory")) {
        /* failure.md, *Out of memory*: a bridge made, a module loaded, run, reloaded and
         * unloaded, and the bridge destroyed - once for every allocation all of that makes,
         * with that one answered null. Each time something is refused and says why, and
         * nothing is written through a null pointer. */
        size_t k, silent = 0;
        (void)bridge_destroy(b);
        for (k = 1;; k ++) {
            if (argc > 2) fprintf(stderr, "allocation %u\n", (unsigned)k);  /* which one, to find a crash */
            asked = 0;
            fail_at = k;
            failed = false;
            said_memory = 0;
            b = bridge_create(&cfg, listening, nullptr);
            if (b) {
                a = store_module(NSTORE_PATH, 6, 1 << 20, 1 << 20);
                a.cfg.reloadable = a.cfg.hot_reload = BRIDGE_TRUE;
                if (!bridge_load(b, &a)) {
                    (void)bridge_run_phase(b, "spawn");
                    (void)bridge_run_phase(b, "attach_a");
                    (void)bridge_run_phase(b, "attach_b_half");
                    (void)bridge_reload(b, &a);
                    (void)bridge_run_phase(b, "look");
                    (void)bridge_unload(b, a.cfg.identity);
                }
                (void)bridge_destroy(b);
            }
            if (!failed) break;
            silent += !said_memory;
            if (argc > 2 && !said_memory) fprintf(stderr, "unsaid\n");
        }
        fail_at = 0;
        printf("%s allocations were each answered null in turn\n", k > 40 ? "more than forty" : "TOO FEW");
        if (silent) printf("FAIL: %u of them were not said\n", (unsigned)silent);
        printf("alive\n");
        return 0;
    }
#endif
    if (!strcmp(what, "shape")) {
        /* backend.md, *Memory is adopted, not created*: a native module's private block
         * reserves what it uses, though the wasm backend is there and wants four gigabytes */
        a = store_module(NSTORE_PATH, 6, 1 << 20, 1 << 20);
        if (bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
        printf("the shared block reserves more than it uses: %s\n",
            b->shared_reserve > b->shared_len ? "yes" : "NO");
        printf("a native module's private block reserves what it uses: %s\n",
            b->modules->priv_reserve == b->modules->priv_len ? "yes" : "NO");
        /* and each is as long as who maps it needs: the shared block whole wasm pages, since
         * the wasm backend maps it, and a native module's private block not */
        printf("the shared block is whole wasm pages: %s\n", b->shared_len % 65'536 ? "NO" : "yes");
        printf("a native module's private block is not rounded to one: %s\n",
            b->modules->priv_len % 65'536 ? "yes" : "NO");
    } else if (!strcmp(what, "changes")) {
        a = store_module(NSTORE_PATH, 6, 1 << 20, 1 << 20);
        if (bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
        run(b, "spawn");
        run(b, "attach_a");         /* six entities gain A */
        run(b, "look");
        run(b, "value_a");          /* one of them is given a value */
        run(b, "look");
        run(b, "attach_b_half");    /* three gain B: a second table */
        run(b, "look");
        run(b, "attach_a");         /* A again, on all six: a set, and no move */
        run(b, "detach_b_absent");  /* a component it does not have */
        run(b, "flicker");          /* there and gone, gone and back, inside one barrier */
        run(b, "look");
        run(b, "detach_b_half");    /* the second table is emptied */
        run(b, "look");
        run(b, "destroy_half");     /* three are destroyed, each twice */
        run(b, "look");
        run(b, "respawn_half");     /* and three made in their place */
        run(b, "touch_dead");       /* changes asked of an id that is gone */
        run(b, "look");
        /* ids are set aside a barrier ahead, so a freed index comes back some rounds later */
        for (int round = 0; round < 3; round ++) {
            run(b, "destroy_half");
            run(b, "respawn_half");
        }
        run(b, "recycled");
        run(b, "attach_a_half");
        run(b, "attach_b");         /* all six carry both */
        run(b, "look");
        run(b, "destroy");          /* and all six go, with both on them */
        run(b, "look");
    } else if (!strcmp(what, "tables")) {
        a = store_module(NSTORETAB_PATH, 10'000, 8 << 20, 32 << 20);
        if (bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
        run(b, "spawn");
        run(b, "combine");          /* ten thousand entities, no two with the same set */
        run(b, "look");
        run(b, "attach_a");         /* every one moves to a table that did not exist */
        run(b, "look");
        run(b, "destroy");
        run(b, "look");
    } else if (!strcmp(what, "notables")) {
        a = store_module(NSTORE_PATH, 6, 1 << 20, 1 << 20);
        a.cfg.max_system_failures = 2;
        if (bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
        run(b, "spawn");
        run(b, "attach_a");         /* the one table there may be */
        run(b, "attach_b_half");    /* a second: three entities stay as they were, and are not heard */
        run(b, "look");             /* a barrier it changed nothing in forgives nothing */
        run(b, "attach_b_half");    /* the second: its budget of two */
        run(b, "attach_b_half");    /* and it is stopped */
        run(b, "look");
    } else if (!strcmp(what, "nocolumns")) {
        a = store_module(NSTORE_PATH, 6, 1 << 20, 1 << 20);
        a.cfg.max_system_failures = 4;
        if (bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
        run(b, "spawn");
        run(b, "attach_a");         /* sixty-four bytes hold four rows of it and not six: three */
        run(b, "look");             /* fit by halves, and four by going back up. It runs on */
        run(b, "attach_a");         /* a value for those four, and the other two are left again */
        run(b, "look");
        run(b, "attach_a");
        run(b, "look");
        run(b, "attach_a");         /* and the other two are never to be had: the fourth barrier */
        run(b, "attach_a");         /* that left them out was its budget of four, and it is stopped */
    } else if (!strcmp(what, "scribble")) {
        a = store_module(NSTORE_PATH, 6, 1 << 20, 1 << 20);
        if (bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
        run(b, "spawn");
        run(b, "attach_a");
        run(b, "scribble");         /* every copy in the block names the first entity */
        run(b, "destroy_half");     /* rows leave, and the last is moved into each one's place */
        run(b, "look");
        run(b, "attach_b_half");    /* the three that are left move to another table */
        run(b, "look");
        run(b, "find");             /* and each is where the index says, under its own id */
    } else if (!strcmp(what, "strip")) {
        a = store_module(NSTORE_PATH, 6, 1 << 20, 1 << 20);
        if (bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
        run(b, "spawn");
        run(b, "attach_a");
        run(b, "value_a");
        run(b, "value_b");          /* three with A, three with A and B */
        strip(b, "st:B");           /* a table of what is left stands, with room: they join it */
        run(b, "look");
        run(b, "look_b");
        run(b, "value_b");          /* and the same two tables again */
        strip(b, "st:A");           /* no table of B alone: the one of A and B becomes it, where */
        run(b, "look");             /* it stands, and three entities are left with nothing */
        run(b, "look_b");           /* the column of B is where the index and the span say */
        run(b, "find");
    } else if (!strcmp(what, "stripfull")) {
        a = store_module(NSTORE_PATH, 6, 1 << 20, 1 << 20);
        if (bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
        run(b, "spawn");
        run(b, "value_b");          /* B first, so that the table of A alone is made with */
        run(b, "attach_a");         /* room for its three rows and the region is then full */
        run(b, "look");
        strip(b, "st:B");           /* the table of A has no room for three more and cannot */
        run(b, "look");             /* grow: two tables of A, and every entity still there */
        run(b, "find");
        run(b, "attach_a");         /* a value for each, which moves none of them */
        run(b, "look");
        run(b, "destroy");          /* and both tables go when they empty */
        run(b, "look");
    } else if (!strcmp(what, "layout")) {
        static const char *const b_gone[1] = { "st:X.b" }, *const arr_cut[1] = { "st:X.arr" };
        static const char *const y_gone[1] = { "st:X.at.y" };
        (void)bridge_destroy(b);
        cfg.budget.sparse_sets = cfg.budget.direct_indexed = 2;
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT1_PATH, "a field added", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT2_PATH, "the fields in another order", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT3_PATH, "an array made longer", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT4_PATH, "a field removed", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT4_PATH, "a field removed, and that accepted", b_gone, 1);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT5_PATH, "an array made shorter", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT5_PATH, "an array made shorter, and that accepted", arr_cut, 1);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT6_PATH, "a field renamed", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT6_PATH, "a field renamed, and the old one's loss accepted", b_gone, 1);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT7_PATH, "a field of another type", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT8_PATH, "a struct a field names, changed", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT11_PATH, "a struct a field names, with a field removed", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT11_PATH, "a struct a field names, with a field removed, and that accepted", y_gone, 1);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT9_PATH, "an enum whose constants have other numbers", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT10_PATH, "an enum with a constant added", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT12_PATH, "an enum with a constant removed", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0_PATH, NLAYOUT13_PATH, "a struct declared as a tag", nullptr, BRIDGE_NONE);
        {
            /* a struct another module's component has a field of: that module's rows hold it */
            bridge_artifact_t holder = store_module(NHOLDER_PATH, BRIDGE_NONE, 1 << 20, BRIDGE_NONE);
            b = bridge_create(&cfg, sink, nullptr);
            a = store_module(NLAYOUT0_PATH, 8, 1 << 20, 4096);
            a.cfg.reloadable = a.cfg.hot_reload = BRIDGE_TRUE;
            holder.cfg.identity = "st:holder@0.1.0";
            printf("a struct a field of another module's component names\n");
            if (!b || bridge_load(b, &a) || bridge_load(b, &holder)) { printf("FAIL: not loaded\n"); return 1; }
            a.path = NLAYOUT8_PATH;
            printf("  reload: %s\n", bridge_reload(b, &a) ? "refused" : "done");
            (void)bridge_destroy(b);
        }
        /* and in each of the other storage layouts, a field added */
        relaid_case(cfg, NLAYOUT0K2_PATH, NLAYOUT1K2_PATH, "a field added, in a sparse set", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0K3_PATH, NLAYOUT1K3_PATH, "a field added, in a toggle", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0K4_PATH, NLAYOUT1K4_PATH, "a field added, direct-indexed", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0K5_PATH, NLAYOUT1K5_PATH, "a field added, in a singleton", nullptr, BRIDGE_NONE);
        /* and the fields in another order */
        relaid_case(cfg, NLAYOUT0K2_PATH, NLAYOUT2K2_PATH, "the fields in another order, in a sparse set", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0K3_PATH, NLAYOUT2K3_PATH, "the fields in another order, in a toggle", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0K4_PATH, NLAYOUT2K4_PATH, "the fields in another order, direct-indexed", nullptr, BRIDGE_NONE);
        relaid_case(cfg, NLAYOUT0K5_PATH, NLAYOUT2K5_PATH, "the fields in another order, in a singleton", nullptr, BRIDGE_NONE);
        b = bridge_create(&cfg, sink, nullptr);
    } else if (!strcmp(what, "move")) {
        static const char *const x[1] = { "st:X" };
        (void)bridge_destroy(b);
        cfg.budget.sparse_sets = cfg.budget.direct_indexed = 1;
        /* every one of the four that an entity carries, to every other */
        for (uint32_t from = 1; from <= 4; from ++) {
            for (uint32_t to = 1; to <= 4; to ++) {
                if (from != to) moved_case(cfg, from, to, "", nullptr, BRIDGE_NONE);
            }
        }
        /* to a singleton: from one entity; from none; and from three, which nothing chooses among */
        moved_case(cfg, 1, 5, "detach_b", nullptr, BRIDGE_NONE);
        moved_case(cfg, 2, 5, "detach_b", nullptr, BRIDGE_NONE);
        moved_case(cfg, 3, 5, "detach_b_half", nullptr, BRIDGE_NONE);
        moved_case(cfg, 4, 5, "", nullptr, BRIDGE_NONE);
        /* and from one, which no entity is there to carry */
        moved_case(cfg, 5, 1, "", nullptr, BRIDGE_NONE);
        /* a toggle with a row that is off: refused, naming the loss, until it is accepted */
        moved_case(cfg, 3, 1, "flicker", nullptr, BRIDGE_NONE);
        moved_case(cfg, 3, 1, "flicker", x, 1);
        /* a loss that is accepted and not made */
        moved_case(cfg, 1, 2, "", x, 1);
        /* no room for the rows where they are to go: nothing has moved */
        cfg.budget.columns = 112;
        moved_case(cfg, 1, 2, "", nullptr, BRIDGE_NONE);
        b = bridge_create(&cfg, sink, nullptr);
    } else if (!strcmp(what, "tag")) {
        /* storage.md, *A tag*: one in each storage layout */
        a = store_module(NTAG_PATH, 8, 1 << 20, 4096);
        if (bridge_load(b, &a)) { printf("not loaded\nalive\n"); return 0; }
        run(b, "spawn");
        run(b, "look");
        run(b, "flicker");          /* the toggled tag turned off for three */
        run(b, "attach_a");         /* the singleton given */
        run(b, "look");
        run(b, "detach_b");         /* the tag kept in tables taken: one table of them again */
        run(b, "look");
        printf("columns in use %u bytes\n", b->column_space.used);
    } else if (!strncmp(what, "single", 6)) {
        /* storage.md, *A singleton* */
        const char *path = !strcmp(what, "singlebad1") ? NSINGLEBAD1_PATH
                         : !strcmp(what, "singlebad2") ? NSINGLEBAD2_PATH
                         : !strcmp(what, "singlebad3") ? NSINGLEBAD3_PATH : NSINGLE_PATH;
        a = store_module(path, 8, 1 << 20, 4096);
        if (bridge_load(b, &a)) { printf("not loaded\nalive\n"); return 0; }
        run(b, "look");             /* declared, and with no value */
        run(b, "attach_a");         /* given one: heard, with the id 0 */
        run(b, "look");
        if (strcmp(what, "single")) { printf("alive\n"); return 0; }
        run(b, "value_a");          /* another value: heard as an attach */
        run(b, "look");
        run(b, "work");             /* a field written where it is: nothing is heard */
        run(b, "look");
        run(b, "detach_b");         /* taken */
        run(b, "look");
        run(b, "attach_a");         /* and given again: zero but for what the attach wrote */
        run(b, "look");
        drop(b, "st:G", BRIDGE_DELETE);     /* lifetime.md: unloaded, it has no value */
        run(b, "look");
    } else if (!strncmp(what, "direct", 6)) {
        a = store_module(!strcmp(what, "directbad") ? NDIRECTBAD_PATH : NDIRECT_PATH, 8, 1 << 20, 4096);
        if (bridge_load(b, &a)) { printf("not loaded\nalive\n"); return 0; }
        run(b, "spawn");
        run(b, "attach_b");         /* three of six are given one, and are heard */
        run(b, "look_b");
        run(b, "near");
        if (!strcmp(what, "direct")) {
            run(b, "value_b");          /* a value for one that has it: heard as an attach */
            run(b, "near");
            run(b, "detach_b_half");    /* one loses it */
            run(b, "look_b");
            run(b, "destroy_half");     /* one that has it, and one that has not */
            run(b, "look_b");
            run(b, "near");
            run(b, "respawn_half");     /* two more, in the places of those two: with no D */
            run(b, "look_b");
            run(b, "near");
            run(b, "detach_b");
            run(b, "look_b");
            run(b, "destroy");
            run(b, "look_b");
        } else {
            /* lifetime.md, *What unload leaves behind*, of a direct-indexed component */
            drop(b, "st:A", BRIDGE_STRIP);      /* so that some carry D and nothing else */
            drop(b, "st:D", !strcmp(what, "directstrip") ? BRIDGE_STRIP
                          : !strcmp(what, "directsweep") ? BRIDGE_SWEEP : BRIDGE_DELETE);
            for (uint32_t e = 1; e <= 6; e ++) {
                printf("entity %u is %s\n", e, storage_is_alive(b->storage, (uint64_t)1 << 32 | e) ? "alive" : "gone");
            }
            run(b, "near");
        }
    } else if (!strcmp(what, "togglefull")) {
        /* budgets.md, *What running out does*, of a table with a toggled column: its bits and
         * the bytes that say where they are need room too, and four of six fit */
        a = store_module(NTOGGLE_PATH, 16, 1 << 20, 4096);
        a.cfg.max_system_failures = 4;
        if (bridge_load(b, &a)) { printf("not loaded\nalive\n"); return 0; }
        run(b, "spawn");
        run(b, "look");
        run(b, "flicker");
        run(b, "look");
    } else if (!strcmp(what, "toggle")) {
        /* storage.md, *A toggle* */
        a = store_module(NTOGGLE_PATH, 16, 1 << 20, 4096);
        if (bridge_load(b, &a)) { printf("not loaded\nalive\n"); return 0; }
        run(b, "spawn");            /* six, each on when it is given */
        run(b, "look");
        run(b, "flicker");          /* the even ones off: a write, and nothing is heard */
        run(b, "look");
        run(b, "look_b");
        run(b, "find");
        run(b, "attach_b");         /* three move to another table, on or off as they were */
        run(b, "look");
        run(b, "find");
        run(b, "value_a");          /* a value for one that is off: it has it, and stays off */
        run(b, "look");
        run(b, "find");
        run(b, "destroy_half");     /* the last row of each table takes the place, bit and all */
        run(b, "look");
        run(b, "find");
        run(b, "respawn_half");     /* six more, on: the table moves to grow, bits and all */
        run(b, "look");
        run(b, "find");
        strip(b, "st:A");           /* the column before it goes, and T and its bits close up */
        run(b, "look");
        run(b, "find");
        run(b, "combine");          /* every one on */
        run(b, "look");
        strip(b, "st:T");
        run(b, "look");
    } else if (!strncmp(what, "sparse", 6)) {
        const char *path = !strcmp(what, "sparsebad1") ? NSPARSEBAD1_PATH
                         : !strcmp(what, "sparsebad2") ? NSPARSEBAD2_PATH : NSPARSE_PATH;
        a = store_module(path, 8, 1 << 20, 4096);
        /* one barrier may leave an entity out, and the module is looked at after it */
        if (!strcmp(what, "sparsefull")) a.cfg.max_system_failures = 4;
        if (bridge_load(b, &a)) { printf("not loaded\nalive\n"); return 0; }
        run(b, "spawn");
        run(b, "attach_b");         /* three of six join the set, and are heard */
        run(b, "look");
        run(b, "look_b");
        if (!strcmp(what, "sparsefull")) {
            /* budgets.md, *What running out does*: the set had room for two of the three */
        } else if (!strcmp(what, "sparse")) {
            run(b, "value_b");          /* a value for one that is in it: heard as an attach */
            run(b, "look");
            run(b, "detach_b_half");    /* one leaves, and the set's last row takes its place */
            run(b, "look");
            run(b, "look_b");
            run(b, "attach_b_half");    /* an entity in the set and in no table */
            run(b, "look");
            run(b, "look_b");
            run(b, "destroy_half");     /* one that is in the set, and one that is not */
            run(b, "look");
            run(b, "look_b");
            run(b, "detach_b");         /* the rest leave, and the lone one stays */
            run(b, "look");
            run(b, "destroy");
            run(b, "look");
            run(b, "look_b");
        } else if (!strcmp(what, "sparsesweepa")) {
            /* a sweep of a component in a table: an entity that is in a sparse set carries
             * something still, and stays, in no table */
            drop(b, "st:A", BRIDGE_SWEEP);
            run(b, "look");
            run(b, "look_b");
        } else {
            /* lifetime.md, *What unload leaves behind*, of a component in a sparse set */
            run(b, "attach_b_half");
            drop(b, "st:S", !strcmp(what, "sparsestrip") ? BRIDGE_STRIP
                          : !strcmp(what, "sparsesweep") ? BRIDGE_SWEEP : BRIDGE_DELETE);
            /* the seventh entity made is the one that carried S and nothing else */
            printf("the entity that was in the set alone is %s\n",
                storage_is_alive(b->storage, (uint64_t)1 << 32 | 7) ? "alive" : "gone");
            run(b, "look");
            run(b, "look_b");
            run(b, "attach_b");         /* and it is a set that can be joined again */
            run(b, "look");
        }
    } else if (!strcmp(what, "stress")) {
        if (stress_load(b)) { printf("FAIL: not loaded\n"); return 1; }
        stress_run(b);
    } else if (!strcmp(what, "weigh")) {
        static const struct { const char *path; uint32_t bytes, count; } fat[] = {
            { NSTOREBIG_PATH, 4, 100'000 },     { NSTOREFAT64_PATH, 64, 100'000 },
            { NSTOREFAT256_PATH, 256, 100'000 }, { NSTOREFAT1K_PATH, 1024, 10'000 },
            { NSTOREFAT4K_PATH, 4096, 10'000 },
        };
        static const struct { const char *path; uint32_t tables; } split[] = {
            { NFRAG1_PATH, 1 }, { NFRAG16_PATH, 16 }, { NFRAG256_PATH, 256 }, { NFRAG4096_PATH, 4096 },
        };
        constexpr uint32_t ROUNDS = 8, ROWS = 204'800;
        (void)bridge_destroy(b);
        cfg.budget.columns = 256 << 20;

        /* a small component given to, and taken from, an entity that carries more and more:
         * the entity moves to another table each time, and all it carries moves with it */
        for (uint32_t k = 0; k < sizeof fat / sizeof *fat; k ++) {
            uint64_t on = 0, off = 0, t;
            b = bridge_create(&cfg, sink, nullptr);
            a = store_module(fat[k].path, fat[k].count, 1 << 20, (int64_t)fat[k].count * 4200);
            if (!b || bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
            (void)barrier(b, "spawn");
            (void)barrier(b, "attach_a");
            for (uint32_t round = 0; round < ROUNDS; round ++) {
                t = barrier(b, "attach_b");
                if (!round || t < on) on = t;
                t = barrier(b, "detach_b");
                if (!round || t < off) off = t;
            }
            printf("an entity of %4u bytes: a component attached %6.1f ns, detached %6.1f ns\n",
                fat[k].bytes, (double)on / fat[k].count, (double)off / fat[k].count);
            (void)bridge_destroy(b);
        }

        /* one system reading one component of the same rows, in one table and in thousands */
        for (uint32_t k = 0; k < sizeof split / sizeof *split; k ++) {
            uint64_t best = 0, t;
            b = bridge_create(&cfg, counted, nullptr);
            a = store_module(split[k].path, ROWS, 8 << 20, 128 << 20);
            if (!b || bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
            (void)barrier(b, "spawn");
            for (uint32_t round = 0; round < ROUNDS * 4; round ++) {
                t = ran(b, "look");
                if (!round || t < best) best = t;
            }
            if (said[0] != ROWS || said[1] != split[k].tables) {
                printf("FAIL: it read %u rows in %u spans\n", said[0], said[1]);
                return 1;
            }
            printf("%u rows in %4u table%s: %6.1f us for all of them, %.3f ns a row\n", ROWS,
                split[k].tables, split[k].tables > 1 ? "s" : " ", (double)best / 1000.0,
                (double)best / ROWS);
            if (k + 1 < sizeof split / sizeof *split) (void)bridge_destroy(b);
        }
        /* storage.md, *A sparse set*: the same change, of a component kept in a sparse set */
        static const struct { const char *path; uint32_t bytes, count; } set[] = {
            { NSPARSEBIG_PATH, 4, 204'800 }, { NSPARSEFAT_PATH, 4096, 10'000 },
        };
        (void)bridge_destroy(b);
        for (uint32_t k = 0; k < sizeof set / sizeof *set; k ++) {
            uint64_t on = 0, off = 0, walk = 0, reach = 0, on_call = 0, off_call = 0, t;
            b = bridge_create(&cfg, counted, nullptr);
            a = store_module(set[k].path, set[k].count, 8 << 20, (int64_t)set[k].count * 4200);
            if (!b || bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
            (void)barrier(b, "spawn");
            for (uint32_t round = 0; round < ROUNDS; round ++) {
                t = barrier(b, "attach_b");
                if (!round || t < on) on = t;
                if (!round || called < on_call) on_call = called;
                if (round + 1 < ROUNDS) {
                    t = barrier(b, "detach_b");
                    if (round < 1 || t < off) off = t;
                    if (round < 1 || called < off_call) off_call = called;
                }
            }
            printf("a sparse set, an entity of %4u bytes: a component given %6.1f ns, taken %6.1f ns; "
                "and the calls %5.1f and %5.1f\n", set[k].bytes, (double)on / set[k].count,
                (double)off / set[k].count, (double)on_call / set[k].count, (double)off_call / set[k].count);
            /* read, where the table's own rows are small: a row of 4 KB is a cache miss by
             * itself, and that is the table's and says nothing of the set */
            for (uint32_t round = 0; !k && round < ROUNDS * 4; round ++) {
                t = ran(b, "look");
                if (!round || t < walk) walk = t;
                t = ran(b, "look_b");
                if (!round || t < reach) reach = t;
            }
            if (!k) {
                printf("    %u of its values walked %6.1f us; reached from a table's rows %6.1f us, "
                    "%.3f ns a row\n", set[k].count, (double)walk / 1000.0, (double)reach / 1000.0,
                    (double)reach / set[k].count);
            }
            if (k + 1 < sizeof set / sizeof *set) (void)bridge_destroy(b);
        }
        /* storage.md, *A toggle*: rows read through their bits, by the share that is on */
        static const struct { const char *path; uint32_t on; } toggled[] = {
            { NTOGGLE1_PATH, 1 }, { NTOGGLE2_PATH, 2 }, { NTOGGLE16_PATH, 16 },
        };
        (void)bridge_destroy(b);
        for (uint32_t k = 0; k < sizeof toggled / sizeof *toggled; k ++) {
            uint64_t words = 0, rows = 0, t;
            b = bridge_create(&cfg, counted, nullptr);
            a = store_module(toggled[k].path, 204'800, 8 << 20, 128 << 20);
            if (!b || bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
            (void)barrier(b, "spawn");
            (void)ran(b, "flicker");
            for (uint32_t round = 0; round < ROUNDS * 4; round ++) {
                t = ran(b, "look_b");
                if (!round || t < rows) rows = t;
                t = ran(b, "look");
                if (!round || t < words) words = t;
            }
            printf("a toggle, one row in %2u on: tested at every row %6.1f us, the rows that are on "
                "found a word at a time %6.1f us\n", toggled[k].on, (double)rows / 1000.0,
                (double)words / 1000.0);
            if (k + 1 < sizeof toggled / sizeof *toggled) (void)bridge_destroy(b);
        }
        /* storage.md, *Direct-indexed*: a value reached by an id that is held */
        (void)bridge_destroy(b);
        {
            uint64_t on = 0, off = 0, near = 0, far = 0, on_call = 0, off_call = 0, t;
            b = bridge_create(&cfg, counted, nullptr);
            a = store_module(NDIRECTBIG_PATH, 204'802, 8 << 20, 128 << 20);
            if (!b || bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
            (void)barrier(b, "spawn");
            for (uint32_t round = 0; round < ROUNDS; round ++) {
                t = barrier(b, "attach_b");
                if (!round || t < on) on = t;
                if (!round || called < on_call) on_call = called;
                if (round + 1 < ROUNDS) {
                    t = barrier(b, "detach_b");
                    if (round < 1 || t < off) off = t;
                    if (round < 1 || called < off_call) off_call = called;
                }
            }
            for (uint32_t round = 0; round < ROUNDS * 4; round ++) {
                t = ran(b, "near");
                if (!round || t < near) near = t;
                t = ran(b, "far");
                if (!round || t < far) far = t;
            }
            printf("direct-indexed: a component given %6.1f ns, taken %6.1f ns; and the calls %5.1f "
                "and %5.1f\n", (double)on / 204'800, (double)off / 204'800, (double)on_call / 204'800,
                (double)off_call / 204'800);
            printf("    204800 values reached by id, in the order they lie %6.3f ns each, in no order "
                "%6.3f ns each\n", (double)near / 204'800, (double)far / 204'800);
        }
        /* storage.md, *A tag*: what a marker costs of [budget] columns, with bytes and with none */
        (void)bridge_destroy(b);
        {
            static const char *const built[2] = { NFRAG4096_PATH, NFRAGTAG4096_PATH };
            uint32_t used[2];
            for (uint32_t k = 0; k < 2; k ++) {
                b = bridge_create(&cfg, counted, nullptr);
                a = store_module(built[k], ROWS, 8 << 20, 128 << 20);
                if (!b || bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
                (void)barrier(b, "spawn");
                used[k] = b->column_space.used >> 10;
                if (!k) (void)bridge_destroy(b);
            }
            printf("204800 entities with six markers each: columns in use %u KB as components of four "
                "bytes, %u KB as tags\n", used[0], used[1]);
        }
        /* storage.md, *A singleton* and *A toggle*: read where it is; and a row switched */
        (void)bridge_destroy(b);
        {
            uint64_t best = 0, t;
            b = bridge_create(&cfg, counted, nullptr);
            a = store_module(NSINGLE_PATH, 8, 1 << 20, 4096);
            if (!b || bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
            (void)barrier(b, "attach_a");
            for (uint32_t round = 0; round < ROUNDS * 4; round ++) {
                t = ran(b, "one");
                if (!round || t < best) best = t;
            }
            if (said[0] != 10'000'000) { printf("FAIL: it read %u\n", said[0]); return 1; }
            printf("a singleton read where it is: %6.3f ns each time\n", (double)best / 1'000'000);
            (void)bridge_destroy(b);
            b = bridge_create(&cfg, counted, nullptr);
            a = store_module(NTOGGLE2_PATH, 204'800, 8 << 20, 128 << 20);
            if (!b || bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
            (void)barrier(b, "spawn");
            for (uint32_t round = 0; round < ROUNDS * 4; round ++) {
                t = ran(b, "flicker");
                if (!round || t < best) best = t;
            }
            printf("a toggled row switched by a write, with the draw that says which way: %6.2f ns a "
                "row\n", (double)best / 204'800);
        }
    } else if (!strcmp(what, "baseline")) {
        static const struct { const char *path; uint32_t on; } built[] = {
            { NWEIGH1_PATH, 1 }, { NWEIGH2_PATH, 2 }, { NWEIGH16_PATH, 16 },
        };
        static const struct { const char *phase, *what; } work[] = {
            { "walk", "read in order" },
            { "near", "reached by id, in the order they lie" },
            { "far",  "reached by id, in no order" },
        };
        constexpr uint32_t ROUNDS = 32, ROWS = 204'800, ASKED = 1'000'000;
        (void)bridge_destroy(b);
        cfg.budget.columns = 256 << 20;
        for (uint32_t k = 0; k < sizeof built / sizeof *built; k ++) {
            uint64_t best = 0, t;
            b = bridge_create(&cfg, counted, nullptr);
            a = store_module(built[k].path, ROWS + 1, 8 << 20, 128 << 20);
            if (!b || bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
            (void)barrier(b, "spawn");
            /* the three that do not depend on how many rows are switched on, once */
            for (uint32_t w = 0; !k && w < sizeof work / sizeof *work; w ++) {
                for (uint32_t round = 0; round < ROUNDS; round ++) {
                    t = ran(b, work[w].phase);
                    if (!round || t < best) best = t;
                }
                if (said[0] != ROWS) { printf("FAIL: it read %u\n", said[0]); return 1; }
                printf("%u values %-38s %7.1f us, %6.3f ns each\n", ROWS, work[w].what,
                    (double)best / 1000.0, (double)best / ROWS);
            }
            for (uint32_t round = 0; round < ROUNDS; round ++) {
                t = ran(b, "flag");
                if (!round || t < best) best = t;
            }
            printf("%u rows tested by a field, %6u of them on  %7.1f us, %6.3f ns a row\n", ROWS,
                said[0], (double)best / 1000.0, (double)best / ROWS);
            if (!k) {
                for (uint32_t round = 0; round < ROUNDS; round ++) {
                    t = ran(b, "one");
                    if (!round || t < best) best = t;
                }
                if (said[0] != ASKED) { printf("FAIL: it read %u\n", said[0]); return 1; }
                printf("one value on one entity, found through its query  %6.3f ns each time\n",
                    (double)best / ASKED);
            }
            if (k + 1 < sizeof built / sizeof *built) (void)bridge_destroy(b);
        }
    } else if (!strcmp(what, "measure")) {
        static const struct { const char *phase, *what; } ops[] = {
            { "spawn",    "an id handed out" },
            { "attach_a", "attach, the entity's first component" },
            { "attach_a", "attach, a component it has" },
            { "attach_b", "attach, a second component" },
            { "detach_b", "detach" },
            { "destroy",  "destroy" },
            { "spawn",    nullptr },
            { "attach_both", "attach, two components in one barrier" },
            { "destroy",  nullptr },
            { "never",    "a barrier with nothing to do" },
        };
        constexpr uint32_t OPS = sizeof ops / sizeof *ops, ROUNDS = 6, COUNT = 100'000;
        uint64_t first[OPS], best[OPS], calls[OPS], t;
        a = store_module(NSTOREBIG_PATH, COUNT, 1 << 20, 64 << 20);
        if (bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
        for (uint32_t round = 0; round < ROUNDS; round ++) {
            for (uint32_t i = 0; i < OPS; i ++) {
                t = barrier(b, ops[i].phase);
                if (!round) first[i] = best[i] = t;
                else if (t < best[i]) best[i] = t;
                if (!round || called < calls[i]) calls[i] = called;
            }
        }
        printf("%u entities, the best of %u rounds, and the first, where tables are new; and what the "
            "module's own call cost\n", COUNT, ROUNDS);
        for (uint32_t i = 0; i < OPS - 1; i ++) {
            if (ops[i].what) printf("%-38s %7.1f ns each   first %7.1f   call %5.1f\n", ops[i].what,
                (double)best[i] / COUNT, (double)first[i] / COUNT, (double)calls[i] / COUNT);
        }
        printf("%-38s %7.0f ns\n", ops[OPS - 1].what, (double)best[OPS - 1]);
        /* with every entity carrying both again: a destroyed one holds nothing in either */
        (void)barrier(b, "spawn");
        (void)barrier(b, "attach_both");
        held(b);
        (void)bridge_destroy(b);

        /* and with ten thousand tables standing */
        b = bridge_create(&cfg, sink, nullptr);
        a = store_module(NSTORETAB_PATH, 10'000, 8 << 20, 32 << 20);
        if (!b || bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
        (void)barrier(b, "spawn");
        t = barrier(b, "combine");
        printf("%-38s %7.3f us a table\n", "ten thousand tables made", (double)t / 1000.0 / 10'000);
        best[0] = barrier(b, "never");
        for (uint32_t round = 0; round < ROUNDS; round ++) {
            t = barrier(b, "never");
            if (t < best[0]) best[0] = t;
        }
        printf("%-38s %7.1f us\n", "a barrier with nothing to do, then", (double)best[0] / 1000.0);
        t = barrier(b, "attach_a");
        printf("%-38s %7.3f us a table\n", "ten thousand more, by one attach each", (double)t / 1000.0 / 10'000);
        /* every one of ten thousand sets filled at one barrier and emptied at the next: where a
         * table that lives only while it has rows is made and released each time */
        for (uint32_t round = 0; round < ROUNDS; round ++) {
            t = barrier(b, "attach_b");
            if (!round || t < best[1]) best[1] = t;
            t = barrier(b, "detach_b");
            if (!round || t < best[2]) best[2] = t;
        }
        printf("%-38s %7.3f us a table\n", "ten thousand sets filled", (double)best[1] / 1000.0 / 10'000);
        printf("%-38s %7.3f us a table\n", "and emptied again", (double)best[2] / 1000.0 / 10'000);
        held(b);
        (void)bridge_destroy(b);

        /* and the stress run, on four threads and on one: eight million changes and their
         * checks. Its records go nowhere here: what they say is the stress case's to judge */
        for (uint32_t k = 0; k < 2; k ++) {
            uint32_t threads = k ? 1 : 4;
            cfg.threads = threads;
            b = bridge_create(&cfg, quiet, nullptr);
            if (!b || stress_load(b)) { printf("FAIL: not loaded\n"); return 1; }
            t = os_get_now();
            stress_run(b);
            printf("the stress run, on %u thread%s            %7.0f ms\n", threads, threads > 1 ? "s" : " ",
                (double)(os_get_now() - t) / 1e6);
            held(b);
            if (threads > 1) (void)bridge_destroy(b);
        }
    } else {
        printf("FAIL: no case '%s'\n", what);
        return 1;
    }
    (void)bridge_destroy(b);
    printf("alive\n");
    return 0;
}
