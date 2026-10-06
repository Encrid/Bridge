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
 *                         spread over many tables. docs/plan/STAGE10.md, *The repair*
 *
 * The first two are a transcript and decide nothing themselves: build.py holds what it must
 * say. It is the same whatever stores the rows, which is the point - docs/plan/STAGE10.md.
 * The third is a measurement, and passes by running. From inside, for the barrier alone. */
#include "inside.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* What the process holds, in KB: the block's touched pages and everything outside it. Asked of
 * the operating system, so that it is the same question whatever keeps the rows. The figures of
 * docs/plan/STAGE10.md are taken on Windows, and elsewhere this says nothing. */
#ifdef _WIN32
typedef struct {
    uint32_t cb, faults;
    size_t peak, now, paged_peak, paged, nonpaged_peak, nonpaged, pagefile, pagefile_peak;
} held_counters;
[[gnu::dllimport]] void *GetCurrentProcess();
[[gnu::dllimport]] int K32GetProcessMemoryInfo(void *process, held_counters *counters, uint32_t size);

static void held(const bridge *b)
{
    held_counters c = { .cb = sizeof c };
    if (!K32GetProcessMemoryInfo(GetCurrentProcess(), &c, sizeof c)) return;
    printf("columns in use %u KB, and the process holds %zu KB\n", b->column_space.used >> 10, c.now >> 10);
}
#else
static void held(const bridge *b) { printf("columns in use %u KB\n", b->column_space.used >> 10); }
#endif

static void sink([[maybe_unused]] void *ud, const bridge_record *r)
{
    printf("  %s: %.*s %u %u\n", r->system ? r->system : "-", (int)r->len, r->text, r->arg[0], r->arg[1]);
}

static void quiet([[maybe_unused]] void *ud, [[maybe_unused]] const bridge_record *r) { }

/* what a system said it read: kept, so that a timing is of a loop that ran */
static uint32_t said[2];
static void counted([[maybe_unused]] void *ud, const bridge_record *r)
{
    if (r->system) said[0] = r->arg[0], said[1] = r->arg[1];
    else if (r->level <= BRIDGE_WARN) printf("  -: %.*s\n", (int)r->len, r->text);
}

static bridge_artifact store_module(const char *path, int64_t spawns, uint32_t spans,
                                    int64_t changes)
{
    bridge_artifact a = {};
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
    a.cfg.config = BRIDGE_NO_CONFIG;
    a.backend = "native";
    a.path = path;
    return a;
}

static const char *const phases[] = {
    "spawn", "attach_a", "attach_b", "detach_b", "destroy", "attach_b_half", "detach_b_half",
    "detach_b_absent", "value_a", "flicker", "destroy_half", "respawn_half", "touch_dead",
    "attach_a_half", "recycled", "combine", "look", "never", "work", "check", "attach_both",
    "scribble", "find", "value_b", "look_b"
};
constexpr uint32_t PHASES = sizeof phases / sizeof *phases;

/* one phase and its barrier, named in the transcript */
/* budgets.md, *Nothing is allocated in a phase*. Built with COUNTED, the library's own calls
 * to the allocator are these, and a phase with its barrier is to make none. */
#ifdef COUNTED
static size_t allocations;
void *counted_malloc(size_t n) { allocations ++; return malloc(n); }
void *counted_calloc(size_t n, size_t each) { allocations ++; return calloc(n, each); }
void *counted_realloc(void *p, size_t n) { allocations ++; return realloc(p, n); }
void counted_free(void *p) { allocations ++; free(p); }
#else
constexpr size_t allocations = 0;
#endif

static void run(bridge *b, const char *phase)
{
    size_t before = allocations;
    printf("%s\n", phase);
    if (bridge_phase(b, phase)) printf("  REFUSED\n");
    if (allocations != before) {
        printf("  FAIL: the allocator was called %u times in a phase\n", (unsigned)(allocations - before));
    }
}

/* the systems of one phase, then the barrier alone, timed */
static uint64_t barrier(bridge *b, const char *phase)
{
    uint32_t i;
    uint64_t t;
    for (i = 0; i < PHASES && strcmp(phases[i], phase); i ++) { }
    schedule_run(b, i);
    t = os_now();
    block_barrier(b);
    return os_now() - t;
}

/* What unload does to a component with the policy `strip`, asked of storage directly so that the
 * module is still there to look: lifetime.md, *What unload leaves behind*. */
static void strip(bridge *b, const char *name)
{
    printf("strip %s\n", name);
    for (uint32_t i = 0; i < b->type_count; i ++) {
        if (!strcmp(b->types[i]->name, name)) storage_drop(b->storage, b->types[i]->type, BRIDGE_STRIP);
    }
    /* a barrier, for what was heard and for what a module reads */
    run(b, "never");
}

/* the systems of one phase, timed, and then its barrier */
static uint64_t ran(bridge *b, const char *phase)
{
    uint32_t i;
    uint64_t t;
    for (i = 0; i < PHASES && strcmp(phases[i], phase); i ++) { }
    t = os_now();
    schedule_run(b, i);
    t = os_now() - t;
    block_barrier(b);
    return t;
}

/* Four of nstress, each in its own namespace, a hundred rounds of work and check. */
constexpr uint32_t STRESSED = 4, ROUNDS = 100;

static int stress_load(bridge *b)
{
    static const char *const identities[STRESSED] = {
        "s1:stress@0.1.0", "s2:stress@0.1.0", "s3:stress@0.1.0", "s4:stress@0.1.0"
    };
    static const char *const paths[STRESSED] = {
        NSTRESS1_PATH, NSTRESS2_PATH, NSTRESS3_PATH, NSTRESS4_PATH
    };
    for (uint32_t i = 0; i < STRESSED; i ++) {
        bridge_artifact a = store_module(paths[i], 20'000, 2 << 20, 16 << 20);
        a.cfg.identity = identities[i];
        a.cfg.scratch = 1 << 20;
        if (bridge_load(b, &a)) return 1;
    }
    return 0;
}

static void stress_run(bridge *b)
{
    for (uint32_t round = 0; round < ROUNDS; round ++) {
        if (bridge_phase(b, "work") || bridge_phase(b, "check")) printf("  REFUSED\n");
    }
}

int main(int argc, char **argv)
{
    const char *what = argc > 1 ? argv[1] : "";
    bridge_backend backends[1] = {{ "native", NATIVE_PATH }};
    bridge_config cfg = { .backends = backends, .backend_count = 1, .phases = phases,
        .phase_count = PHASES, .threads = 1, .budget =
        { .entities = 400'000, .tables = 32'768, .types = 64, .systems = 64, .terms = 4, .modules = 4,
          .persistent = 4096, .columns = 64 << 20 } };
    bridge *b;

    /* the stress run is the one thing here that is run on more than one thread */
    if (!strcmp(what, "stress")) cfg.threads = 4;
    if (!strcmp(what, "stress1")) what = "stress";      /* the same, on the caller's thread alone */
    if (!strcmp(what, "notables")) cfg.budget.tables = 1;
    if (!strcmp(what, "nocolumns")) cfg.budget.columns = 64;
    if (!strcmp(what, "stripfull")) cfg.budget.columns = 160;
    b = bridge_create(&cfg, sink, nullptr);
    bridge_artifact a;

    setvbuf(stdout, nullptr, _IONBF, 0);
    if (!b) { printf("FAIL: bridge_create\n"); return 1; }

    if (!strcmp(what, "changes")) {
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
        run(b, "attach_a");         /* sixty-four bytes hold five rows of it and not six: three */
        run(b, "look");             /* fit by halves, and five by going back up. It runs on */
        run(b, "attach_a");         /* a value for those five, and the sixth is left again */
        run(b, "look");
        run(b, "attach_a");
        run(b, "look");
        run(b, "attach_a");         /* and the sixth is never to be had: the fourth barrier that */
        run(b, "attach_a");         /* left one out was its budget of four, and it is stopped */
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
        uint64_t first[OPS], best[OPS], t;
        a = store_module(NSTOREBIG_PATH, COUNT, 1 << 20, 64 << 20);
        if (bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
        for (uint32_t round = 0; round < ROUNDS; round ++) {
            for (uint32_t i = 0; i < OPS; i ++) {
                t = barrier(b, ops[i].phase);
                if (!round) first[i] = best[i] = t;
                else if (t < best[i]) best[i] = t;
            }
        }
        printf("%u entities, the best of %u rounds, and the first, where tables are new\n", COUNT, ROUNDS);
        for (uint32_t i = 0; i < OPS - 1; i ++) {
            if (ops[i].what) printf("%-38s %7.1f ns each   first %7.1f\n", ops[i].what,
                (double)best[i] / COUNT, (double)first[i] / COUNT);
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
        printf("%-38s %7.1f us a table\n", "ten thousand tables made", (double)t / 1000.0 / 10'000);
        best[0] = barrier(b, "never");
        for (uint32_t round = 0; round < ROUNDS; round ++) {
            t = barrier(b, "never");
            if (t < best[0]) best[0] = t;
        }
        printf("%-38s %7.1f us\n", "a barrier with nothing to do, then", (double)best[0] / 1000.0);
        t = barrier(b, "attach_a");
        printf("%-38s %7.1f us a table\n", "ten thousand more, by one attach each", (double)t / 1000.0 / 10'000);
        /* every one of ten thousand sets filled at one barrier and emptied at the next: where a
         * table that lives only while it has rows is made and released each time */
        for (uint32_t round = 0; round < ROUNDS; round ++) {
            t = barrier(b, "attach_b");
            if (!round || t < best[1]) best[1] = t;
            t = barrier(b, "detach_b");
            if (!round || t < best[2]) best[2] = t;
        }
        printf("%-38s %7.1f us a table\n", "ten thousand sets filled", (double)best[1] / 1000.0 / 10'000);
        printf("%-38s %7.1f us a table\n", "and emptied again", (double)best[2] / 1000.0 / 10'000);
        held(b);
        (void)bridge_destroy(b);

        /* and the stress run, on four threads and on one: eight million changes and their
         * checks. Its records go nowhere here: what they say is the stress case's to judge */
        for (uint32_t k = 0; k < 2; k ++) {
            uint32_t threads = k ? 1 : 4;
            cfg.threads = threads;
            b = bridge_create(&cfg, quiet, nullptr);
            if (!b || stress_load(b)) { printf("FAIL: not loaded\n"); return 1; }
            t = os_now();
            stress_run(b);
            printf("the stress run, on %u thread%s            %7.0f ms\n", threads, threads > 1 ? "s" : " ",
                (double)(os_now() - t) / 1e6);
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
