/* A host, as far as phase 3: it creates a bridge with storage, loads modules compiled from C,
 * runs phases, and prints every record.
 *
 *     phase3 <case>
 *
 * Each case is one clause of docs/PLAN.md phase 3. ctest checks a case by the records it
 * emits, and every case ends by printing "alive". */
#include "bridge.h"
#include "bridge-abi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MS 1000000ll

static void sink(void *ud, const bridge_record *r)
{
    (void)ud;
    printf("record[%d] %s/%s: %.*s %u %u\n", r->level, r->identity ? r->identity : "-",
        r->system ? r->system : "-", (int)r->len, r->text, r->arg[0], r->arg[1]);
}

static void *read_file(const char *name, size_t *len)
{
    char path[1024];
    FILE *f;
    void *data;
    snprintf(path, sizeof path, "%s/%s.wasm", MODULE_DIR, name);
    f = fopen(path, "rb");
    if (!f) { printf("cannot read %s\n", path); exit(2); }
    fseek(f, 0, SEEK_END);
    *len = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(*len);
    if (fread(data, 1, *len, f) != *len) { printf("cannot read %s\n", path); exit(2); }
    fclose(f);
    return data;
}

/* a module as a deployment would state it: every key said, nothing left at zero */
static bridge_artifact artifact(const char *identity, const char *file)
{
    bridge_artifact a = {0};
    a.cfg.identity = identity;
    a.cfg.abi = BRIDGE_ABI;
    a.cfg.trusted = false;
    a.cfg.on_error = BRIDGE_SKIP;
    a.cfg.fuel = 10000000;
    a.cfg.memory_limit = 16 << 20;
    a.cfg.init_timeout = 2000 * MS;
    a.cfg.log_level = 0;
    a.cfg.scratch = 64 << 10;
    a.cfg.log_ring = 64 * 24;
    a.cfg.spans = 16 << 10;
    /* what each test module mints between two barriers: most mint nothing, and say so */
    a.cfg.spawns = !strcmp(file, "mover") ? 1024 : !strcmp(file, "refs") ? 128
                 : !strcmp(file, "namer") ? 8 : BRIDGE_NONE;
    a.cfg.persistent = BRIDGE_NONE;
    a.backend = "wasm";
    a.bytes = read_file(file, &a.len);
    return a;
}

static int load(bridge *b, const bridge_artifact *a)
{
    int r = bridge_load(b, a);
    printf("%s: %s\n", a->cfg.identity, r ? "not loaded" : "loaded");
    return r;
}

static void frames(bridge *b, int count)
{
    int i;
    for (i = 0; i < count; i ++) {
        if (bridge_phase(b, "update")) printf("update refused\n");
        if (bridge_phase(b, "present")) printf("present refused\n");
    }
}

static const char *const grants[] = { "game:Position", "game:Marked" };

static bridge_artifact reader(void)
{
    bridge_artifact a = artifact("view:reader@0.1.0", "reader");
    a.cfg.read = grants;
    a.cfg.read_count = 2;
    return a;
}

/* validation.md, *Always on*: the accessor's test, against a build without it, with the
 * count that fuel adds and without. Nanoseconds a row, the best of twenty. */
static void bench(bridge_config *cfg)
{
    static const char *const builds[2] = { "rows", "rows-unchecked" };
    int build, metered, round, pass;
    cfg->budget.entities = 262144;
    cfg->budget.columns = 64 << 20;
    for (build = 0; build < 2; build ++) {
        for (metered = 1; metered >= 0; metered --) {
            bridge *b = bridge_create(cfg, sink, NULL);
            bridge_artifact a = artifact("game:rows@0.1.0", builds[build]);
            double best = 1e30;
            a.cfg.fuel = metered ? 1000000000 : BRIDGE_NO_LIMIT;
            a.cfg.init_timeout = BRIDGE_NO_LIMIT;
            a.cfg.spawns = 200000;
            if (!b || bridge_load(b, &a)) { printf("bench: cannot load\n"); exit(2); }
            for (round = 0; round < 20; round ++) {
                struct timespec t0, t1;
                double ns;
                timespec_get(&t0, TIME_UTC);
                for (pass = 0; pass < 50; pass ++) bridge_phase(b, "update");
                timespec_get(&t1, TIME_UTC);
                ns = (double)(t1.tv_sec - t0.tv_sec) * 1e9 + (double)(t1.tv_nsec - t0.tv_nsec);
                if (ns < best) best = ns;
            }
            printf("%-15s %-9s %.3f ns a row\n", builds[build], metered ? "fuel" : "no fuel",
                best / (50.0 * 200000.0));
            bridge_destroy(b);
        }
    }
}

int main(int argc, char **argv)
{
    const char *what = argc > 1 ? argv[1] : "";
    bridge_backend backends[1] = {{ "wasm", BACKEND_PATH }};
    const char *phases[2] = { "update", "present" };
    bridge_config cfg = { backends, 1, phases, 2, NULL, 0, 1, { 4096, 4096, 1 << 20, 8 } };
    bridge_artifact a;
    bridge *b;
    setvbuf(stdout, NULL, _IONBF, 0);

    if (!strcmp(what, "bench")) {
        bench(&cfg);
        printf("alive\n");
        return 0;
    }
    if (!strcmp(what, "nophases")) cfg.phase_count = 0;
    if (!strcmp(what, "nobudget")) cfg.budget.columns = 0;
    if (!strcmp(what, "nocolumns")) cfg.budget.columns = (uint64_t)atoi(argc > 2 ? argv[2] : "0");

    b = bridge_create(&cfg, sink, NULL);
    if (!b) {
        printf("bridge_create refused\nalive\n");
        return 0;
    }

    if (!strcmp(what, "share") || !strcmp(what, "spans") || !strcmp(what, "log")) {
        /* one column, two modules */
        a = artifact("game:mover@0.1.0", "mover");
        load(b, &a);
        a = reader();
        load(b, &a);
        frames(b, 3);
    } else if (!strcmp(what, "mirror")) {
        a = artifact("game:refs@0.1.0", "refs");
        load(b, &a);
        frames(b, 3);
    } else if (!strcmp(what, "fingerprint")) {
        a = artifact("game:mover@0.1.0", "mover");
        load(b, &a);
        a = artifact("game:fp@0.1.0", "fp");
        load(b, &a);
    } else if (!strcmp(what, "grant")) {        /* the reader, with no grant */
        a = artifact("view:reader@0.1.0", "reader");
        load(b, &a);
    } else if (!strcmp(what, "wildcard")) {
        a = artifact("game:wildcard@0.1.0", "wildcard");
        load(b, &a);
    } else if (!strcmp(what, "fuel")) {
        a = artifact("test:spin@0.1.0", "spin3");
        load(b, &a);
        frames(b, 2);
    } else if (!strcmp(what, "ceiling")) {
        /* 3,000 ids a barrier under a ceiling of 4,096: the first frame runs out of what
         * was set aside for it, and the second of what the deployment has */
        a = artifact("test:flood@0.1.0", "flood");
        a.cfg.spawns = 3000;
        load(b, &a);
        frames(b, 2);
    } else if (!strcmp(what, "ring")) {
        a = artifact("test:chatty@0.1.0", "chatty");
        a.cfg.log_ring = 16 * 24;
        load(b, &a);
        frames(b, 1);
    } else if (!strcmp(what, "nospans")) {      /* the reader's spans do not fit; the mover runs on */
        a = artifact("game:mover@0.1.0", "mover");
        load(b, &a);
        a = reader();
        a.cfg.spans = 16;
        load(b, &a);
        frames(b, 1);
    } else if (!strcmp(what, "nocolumns")) {    /* storage cannot hold what the mover spawns */
        a = artifact("game:mover@0.1.0", "mover");
        load(b, &a);
        frames(b, 1);
    } else if (!strcmp(what, "strings") || !strcmp(what, "noarena") || !strcmp(what, "bigarena")) {
        /* a string a component keeps: allocated from the module's share of the arena */
        a = artifact("game:namer@0.1.0", "namer");
        a.cfg.persistent = !strcmp(what, "strings") ? 64 : !strcmp(what, "noarena") ? 4 : 1 << 20;
        load(b, &a);
        frames(b, 1);
    } else if (!strcmp(what, "nospawns")) {     /* the mover, stating that it spawns nothing */
        a = artifact("game:mover@0.1.0", "mover");
        a.cfg.spawns = BRIDGE_NONE;
        load(b, &a);
    } else if (!strcmp(what, "undo")) {
        /* a load that fails after declaring and spawning leaves nothing: the same component
         * name with another layout then loads, which a ghost of the first would refuse */
        a = artifact("game:wildcard@0.1.0", "wildcard");
        load(b, &a);
        a = artifact("game:fp@0.1.0", "fp");
        load(b, &a);
    } else if (!strcmp(what, "nophase")) {
        if (bridge_phase(b, "render")) printf("render refused\n");
    }

    bridge_destroy(b);
    printf("alive\n");
    return 0;
}
