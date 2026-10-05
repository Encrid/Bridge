/* Storage: what it does, said out loud, and what it costs.
 *
 *     storage changes     every structural change a module can ask for, one at a time, and what
 *                         a system and an observer see after each
 *     storage tables      ten thousand tables
 *     storage measure     nanoseconds for each kind of change, at a barrier
 *
 * The first two are a transcript and decide nothing themselves: build.py holds what it must
 * say. It is the same whatever stores the rows, which is the point - docs/plan/PHASE10.md.
 * The third is a measurement, and passes by running. From inside, for the barrier alone. */
#include "inside.h"

#include <stdio.h>
#include <string.h>

static void sink([[maybe_unused]] void *ud, const bridge_record *r)
{
    printf("  %s: %.*s %u %u\n", r->system ? r->system : "-", (int)r->len, r->text, r->arg[0], r->arg[1]);
}

static bridge_artifact store_module(const char *path, int64_t spawns, uint32_t spans)
{
    bridge_artifact a = {};
    a.cfg.identity = "st:store@0.1.0";
    a.cfg.abi = BRIDGE_ABI;
    a.cfg.trusted = true;
    a.cfg.on_error = BRIDGE_FAIL;
    a.cfg.max_system_failures = 1;
    a.cfg.fuel = BRIDGE_NO_LIMIT;
    a.cfg.memory_limit = BRIDGE_NO_LIMIT;
    a.cfg.init_timeout = BRIDGE_NO_LIMIT;
    a.cfg.reload_timeout = BRIDGE_NO_LIMIT;
    a.cfg.shutdown_timeout = BRIDGE_NO_LIMIT;
    a.cfg.scratch = 64 << 10;
    a.cfg.log_ring = 256 * 24;
    a.cfg.spans = spans;
    a.cfg.spawns = spawns;
    a.cfg.persistent = BRIDGE_NONE;
    a.cfg.config = BRIDGE_NO_CONFIG;
    a.backend = "native";
    a.path = path;
    return a;
}

static const char *const phases[] = {
    "spawn", "attach_a", "attach_b", "detach_b", "destroy", "attach_b_half", "detach_b_half",
    "detach_b_absent", "value_a", "flicker", "destroy_half", "respawn_half", "touch_dead",
    "attach_a_half", "recycled", "combine", "look", "never"
};
constexpr uint32_t PHASES = sizeof phases / sizeof *phases;

/* one phase and its barrier, named in the transcript */
static void run(bridge *b, const char *phase)
{
    printf("%s\n", phase);
    if (bridge_phase(b, phase)) printf("  REFUSED\n");
}

/* the systems of one phase, then the barrier alone, timed */
static uint64_t barrier(bridge *b, const char *phase)
{
    uint32_t i;
    uint64_t t;
    for (i = 0; i < PHASES && strcmp(phases[i], phase); i ++) { }
    store_enter(b);
    schedule_run(b, i);
    t = os_now();
    store_barrier(b);
    return os_now() - t;
}

int main(int argc, char **argv)
{
    const char *what = argc > 1 ? argv[1] : "";
    bridge_backend backends[1] = {{ "native", NATIVE_PATH }};
    bridge_config cfg = { backends, 1, phases, PHASES, nullptr, 0, 1, { 400'000, 4096, 64 << 20, 4 } };
    bridge *b = bridge_create(&cfg, sink, nullptr);
    bridge_artifact a;

    setvbuf(stdout, nullptr, _IONBF, 0);
    if (!b) { printf("FAIL: bridge_create\n"); return 1; }

    if (!strcmp(what, "changes")) {
        a = store_module(NSTORE_PATH, 6, 64 << 10);
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
        a = store_module(NSTORETAB_PATH, 10'000, 8 << 20);
        if (bridge_load(b, &a)) { printf("FAIL: not loaded\n"); return 1; }
        run(b, "spawn");
        run(b, "combine");          /* ten thousand entities, no two with the same set */
        run(b, "look");
        run(b, "attach_a");         /* every one moves to a table that did not exist */
        run(b, "look");
        run(b, "destroy");
        run(b, "look");
    } else if (!strcmp(what, "measure")) {
        static const struct { const char *phase, *what; } ops[] = {
            { "spawn",    "an id handed out" },
            { "attach_a", "attach, the entity's first component" },
            { "attach_a", "attach, a component it has" },
            { "attach_b", "attach, a second component" },
            { "detach_b", "detach" },
            { "destroy",  "destroy" },
            { "never",    "a barrier with nothing to do" },
        };
        constexpr uint32_t OPS = sizeof ops / sizeof *ops, ROUNDS = 6, COUNT = 100'000;
        uint64_t first[OPS], best[OPS], t;
        a = store_module(NSTOREBIG_PATH, COUNT, 64 << 10);
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
            printf("%-38s %7.1f ns each   first %7.1f\n", ops[i].what,
                (double)best[i] / COUNT, (double)first[i] / COUNT);
        }
        printf("%-38s %7.0f ns\n", ops[OPS - 1].what, (double)best[OPS - 1]);
        printf("columns in use %u KB\n", b->columns_used >> 10);
        (void)bridge_destroy(b);

        /* and with ten thousand tables standing */
        b = bridge_create(&cfg, sink, nullptr);
        a = store_module(NSTORETAB_PATH, 10'000, 8 << 20);
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
        printf("columns in use %u KB\n", b->columns_used >> 10);
    } else {
        printf("FAIL: no case '%s'\n", what);
        return 1;
    }
    (void)bridge_destroy(b);
    printf("alive\n");
    return 0;
}
