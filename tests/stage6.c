/* A host, as far as stage 6: modules that fail, stop, leave, come back changed, and are
 * configured.
 *
 *     stage6 <case>
 *
 * Each case is one clause of docs/PLAN.md stage 6 or one refusal the phase added, checked by
 * the records it emits and by what bridge_modules answers. */
#include "bridge.h"
#include "bridge-abi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constexpr int64_t MS = 1'000'000;

static uint32_t path_at[4], path_count;
static uint32_t out_of_step;
static int quiet;

static void sink([[maybe_unused]] void *ud, const bridge_record *r)
{
    if (r->len == 7 && !memcmp(r->text, "path at", 7) && path_count < 4) path_at[path_count ++] = r->arg[0];
    if (r->len > 40 && !memcmp(r->text, "the entity index is out of step", 31)) out_of_step ++;
    if (quiet && r->level > BRIDGE_WARN) return;
    printf("record[%d] %s/%s: %.*s %u %u\n", r->level, r->identity ? r->identity : "-",
        r->system ? r->system : "-", (int)r->len, r->text, r->arg[0], r->arg[1]);
}

static bridge_artifact native(const char *identity, const char *path)
{
    bridge_artifact a = {};
    a.cfg.identity = identity;
    a.cfg.abi = BRIDGE_ABI;
    a.cfg.trusted = BRIDGE_TRUE;
    a.cfg.reloadable = BRIDGE_FALSE;
    a.cfg.hot_reload = BRIDGE_FALSE;
    a.cfg.pinned = BRIDGE_FALSE;
    a.cfg.on_error = BRIDGE_SKIP;
    a.cfg.max_system_failures = 3;
    a.cfg.work_limit = BRIDGE_NO_LIMIT;
    a.cfg.memory_limit = BRIDGE_NO_LIMIT;
    a.cfg.init_timeout = BRIDGE_NO_LIMIT;
    a.cfg.reload_timeout = BRIDGE_NO_LIMIT;
    a.cfg.shutdown_timeout = BRIDGE_NO_LIMIT;
    a.cfg.scratch = 64 << 10;
    a.cfg.log_ring = 64 * 24;
    a.cfg.spans = 16 << 10;
    a.cfg.log_level = BRIDGE_INFO;
    a.cfg.spawns = 32;
    a.cfg.persistent = BRIDGE_NONE;
    a.cfg.changes = 1 << 20;
    a.cfg.config = BRIDGE_NO_CONFIG;
    a.backend = "native";
    a.path = path;
    return a;
}

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
    a.cfg.trusted = BRIDGE_FALSE;
    a.cfg.work_limit = 10'000'000;
    a.cfg.memory_limit = 16 << 20;
    a.cfg.init_timeout = 2000 * MS;
    a.cfg.reload_timeout = 2000 * MS;
    a.cfg.shutdown_timeout = 2000 * MS;
    return a;
}

static void load(bridge *b, const bridge_artifact *a)
{
    printf("%s: %s\n", a->cfg.identity, bridge_load(b, a) ? "not loaded" : "loaded");
}

static void reload(bridge *b, const bridge_artifact *a)
{
    printf("%s: %s\n", a->cfg.identity, bridge_reload(b, a) ? "not reloaded" : "reloaded");
}

static void unload(bridge *b, const char *identity)
{
    printf("%s: %s\n", identity, bridge_unload(b, identity) ? "not unloaded" : "unloaded");
}

static void frames(bridge *b, int count)
{
    int i;
    for (i = 0; i < count; i ++) {
        if (bridge_phase(b, "update")) printf("update refused\n");
        if (bridge_phase(b, "present")) printf("present refused\n");
    }
}

/* the pull: what is loaded, and whether it still runs */
static void modules(bridge *b)
{
    bridge_module_info info[8];
    int n = bridge_modules(b, info, 8), i;
    printf("modules:");
    for (i = 0; i < n && i < 8; i ++) {
        printf(" %s=%s", info[i].identity, info[i].state == BRIDGE_STOPPED ? "STOPPED" : "RUNNING");
    }
    printf(" (%d)\n", n);
}

static const char *const position[] = { "game:Position", "game:Roster" };

static bridge_artifact counter()
{
    bridge_artifact a = wasm("view:counter@0.1.0", "counter");
    a.cfg.read = position;
    a.cfg.read_count = 2;
    a.cfg.spawns = BRIDGE_NONE;
    return a;
}

/* a module with every kind of setting, as a deployment would hand them over */
static bridge_setting everything[11] = {
    { .key = "vsync", .kind = BRIDGE_BOOL, .v = { .b = true } },
    { .key = "width", .kind = BRIDGE_INT, .v = { .i = 1280 } },
    { .key = "scale", .kind = BRIDGE_FLOAT, .v = { .f = 1.5 } },
    { .key = "mode", .kind = BRIDGE_STRING, .v = { .s = "Adaptive" } },
    { .key = "title", .kind = BRIDGE_STRING, .v = { .s = "hello" } },
    { .key = "path", .kind = BRIDGE_STRING, .v = { .s = "some/long/path.cfg" } },
    { .key = "small", .kind = BRIDGE_INT, .v = { .i = 7 } },
    { .key = "flags", .kind = BRIDGE_INT, .v = { .i = 3 } },             /* a bitmask, by its bits */
    { .key = "ratio", .kind = BRIDGE_INT, .v = { .i = 3 } },             /* an integer into a float field */
    { .key = "big", .kind = BRIDGE_INT, .v = { .i = 8'589'934'592ll } },  /* more than 32 bits */
    { nullptr, 0, { 0 } },
};

static bridge_artifact configured(const char *identity)
{
    bridge_artifact a = wasm(identity, "settings");
    a.cfg.config = "game:Settings";
    a.cfg.settings = everything;
    a.cfg.setting_count = 10;
    a.cfg.persistent = 64;
    return a;
}

int main(int argc, char **argv)
{
    const char *what = argc > 1 ? argv[1] : "";
    bridge_backend backends[2] = {{ "wasm", BACKEND_PATH }, { "native", NATIVE_PATH }};
    const char *phases[2] = { "update", "present" };
    bridge_config cfg = { .abi = BRIDGE_HOST_ABI, .backends = backends, .backend_count = 2, .phases = phases,
        .phase_count = 2, .threads = 1, .budget =
        { .entities = 4096, .tables = 64, .types = 64, .systems = 128, .terms = 8, .modules = 8,
          .persistent = 4096, .columns = 1 << 20 } };
    bridge_artifact a;
    bridge *b;
    setvbuf(stdout, nullptr, _IONBF, 0);

    if (!strcmp(what, "slots")) cfg.budget.modules = 1;
    /* the ticker has one system: room for it once, and a reload still has room */
    if (!strcmp(what, "reloadexact") || !strcmp(what, "reloadexactfail")) cfg.budget.systems = 1;
    /* room for one module's share and no more: every load of the two hundred takes it, and
     * every unload gives it back - strings.md, *It is given back when nothing points into it* */
    if (!strcmp(what, "churn") || !strncmp(what, "arena", 5)) cfg.budget.persistent = 64;
    if (!strcmp(what, "configceiling")) cfg.budget.entities = (uint64_t)atoi(argc > 2 ? argv[2] : "0");
    b = bridge_create(&cfg, sink, nullptr);
    if (!b) {
        printf("bridge_create refused\nalive\n");
        return 0;
    }

    if (!strcmp(what, "skipinit")) {            /* a failed init, skipped: the rest runs on */
        a = wasm("game:twostep@0.1.0", "twostep");
        load(b, &a);
        a = wasm("game:badobs@0.1.0", "badobs");
        load(b, &a);
        frames(b, 1);
        modules(b);
    } else if (!strcmp(what, "refusedload")) {  /* nothing of a refused load is left */
        a = wasm("game:badterm@0.1.0", "badterm");
        load(b, &a);
        a = wasm("game:fp@0.1.0", "fp");
        load(b, &a);
        modules(b);
    } else if (!strcmp(what, "budget") || !strcmp(what, "failstop") || !strcmp(what, "failgiveup")) {
        a = wasm("game:twostep@0.1.0", "twostep");
        load(b, &a);
        a = wasm("test:flaky@0.1.0", "flaky-always");
        a.cfg.spawns = BRIDGE_NONE;
        if (strcmp(what, "budget")) a.cfg.on_error = BRIDGE_FAIL;
        load(b, &a);
        frames(b, 5);
        modules(b);
        /* a stopped bridge refuses an unload as it refuses a phase and a load */
        if (strcmp(what, "budget")) unload(b, "game:twostep@0.1.0");
        /* and takes an unload of the one that stopped it: the host goes on without it */
        if (!strcmp(what, "failgiveup")) {
            unload(b, "test:flaky@0.1.0");
            frames(b, 1);
            modules(b);
        }
    } else if (!strcmp(what, "forgiven") || !strcmp(what, "notforgiven")) {
        /* one pass in three fails, or two in three: the count falls as fast as it rises */
        a = wasm("test:flaky@0.1.0", !strcmp(what, "forgiven") ? "flaky-third" : "flaky-twothirds");
        a.cfg.spawns = BRIDGE_NONE;
        load(b, &a);
        frames(b, 12);
        modules(b);
    } else if (!strcmp(what, "notmasked")) {
        /* a system that always fails, and one in the next phase that never does: the clean
         * one does not forgive the other */
        a = wasm("test:flaky@0.1.0", "flaky-calm");
        a.cfg.spawns = BRIDGE_NONE;
        load(b, &a);
        frames(b, 5);
        modules(b);
    } else if (!strcmp(what, "neverstop")) {    /* -1: it fails every pass and is never stopped */
        a = wasm("test:flaky@0.1.0", "flaky-always");
        a.cfg.spawns = BRIDGE_NONE;
        a.cfg.max_system_failures = BRIDGE_NO_LIMIT;
        load(b, &a);
        frames(b, 8);
        modules(b);
    } else if (!strcmp(what, "strip") || !strcmp(what, "sweep") || !strcmp(what, "delete")) {
        char file[32];
        snprintf(file, sizeof file, "keeper-%s", what);
        a = wasm("game:keeper@0.1.0", file);
        load(b, &a);
        a = counter();
        load(b, &a);
        frames(b, 1);
        unload(b, "game:keeper@0.1.0");
        frames(b, 1);
        /* the component only the keeper declared is gone: its name is free for another layout */
        a = wasm("game:retype@0.1.0", "retype-private");
        a.cfg.spawns = BRIDGE_NONE;
        load(b, &a);
        modules(b);
    } else if (!strcmp(what, "nested-unload")) {
        a = wasm("game:typer@0.1.0", "typer");
        a.cfg.spawns = BRIDGE_NONE;
        load(b, &a);
        a = wasm("game:user@0.1.0", "user");
        load(b, &a);
        unload(b, "game:typer@0.1.0");          /* the type's declarer goes; its user stays */
        frames(b, 1);
        a = wasm("game:retype@0.1.0", "retype-vec2");
        a.cfg.spawns = BRIDGE_NONE;
        load(b, &a);                            /* refused: the type is still held */
        unload(b, "game:user@0.1.0");
        load(b, &a);                            /* and now it is not */
    } else if (!strcmp(what, "destroy")) {
        /* a bridge destroyed with modules loaded shuts each of them down first */
        a = wasm("game:ticker@0.1.0", "ticker");
        load(b, &a);
        a = wasm("test:hang@0.1.0", "hang");
        a.cfg.spawns = BRIDGE_NONE;
        a.cfg.shutdown_timeout = 100 * MS;
        load(b, &a);
        frames(b, 1);
    } else if (!strcmp(what, "nounload")) {
        unload(b, "game:nobody@0.1.0");
    } else if (!strcmp(what, "timeouts")) {     /* a shutdown that never returns */
        a = wasm("test:hang@0.1.0", "hang");
        a.cfg.spawns = BRIDGE_NONE;
        a.cfg.shutdown_timeout = 100 * MS;
        load(b, &a);
        unload(b, "test:hang@0.1.0");
        modules(b);
    } else if (!strncmp(what, "share", 5)) {
        /* A module with sixteen bytes of the arena, reloaded into one whose reload takes
         * forty. Stating more gives it more; the same leaves it short; less is refused. With
         * another module's share carved after its own, the more is a second range. */
        a = wasm("game:ticker@0.1.0", "ticker");
        a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
        a.cfg.persistent = 16;
        load(b, &a);
        if (!strcmp(what, "sharechain")) {
            a = wasm("game:namer@0.1.0", "namer");
            a.cfg.persistent = 64;
            load(b, &a);
        }
        frames(b, 2);
        a = wasm("game:ticker@0.2.0", "ticker-alloc");
        a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
        a.cfg.persistent = !strcmp(what, "sharesame") ? 16 : !strcmp(what, "shareless") ? 8
                         : !strcmp(what, "sharebig") ? 1 << 20 : 64;
        reload(b, &a);
        frames(b, 2);
        modules(b);
    } else if (!strcmp(what, "repair")) {
        /* an essential module stopped at its budget stops the bridge; reloading it into one
         * that works is the one thing a stopped bridge accepts, and it runs again */
        a = wasm("game:ticker@0.1.0", "ticker-trip");
        a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
        a.cfg.on_error = BRIDGE_FAIL;
        load(b, &a);
        frames(b, 4);
        a = wasm("game:twostep@0.1.0", "twostep");
        load(b, &a);                            /* another module: refused */
        a = wasm("game:ticker@0.2.0", "ticker-ten");
        a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
        a.cfg.on_error = BRIDGE_FAIL;
        reload(b, &a);
        frames(b, 2);
        modules(b);
    } else if (!strcmp(what, "repairload")) {
        /* the same when what stopped it was a load: loading that module again, from an
         * artifact that works, lifts it */
        a = wasm("game:thing@0.1.0", "badobs");
        a.cfg.on_error = BRIDGE_FAIL;
        load(b, &a);
        a = wasm("game:other@0.1.0", "twostep");
        load(b, &a);
        a = wasm("game:thing@0.2.0", "twostep");
        a.cfg.on_error = BRIDGE_FAIL;
        load(b, &a);
        frames(b, 1);
        modules(b);
    } else if (!strcmp(what, "reload") || !strcmp(what, "reloadlayout") || !strcmp(what, "reloadfail")
               || !strcmp(what, "reloadmove") || !strcmp(what, "reloadexact")
               || !strcmp(what, "reloadexactfail") || !strcmp(what, "reloadspawns")
               || !strcmp(what, "reloaddrop") || !strcmp(what, "maynot")) {
        a = wasm("game:ticker@0.1.0", "ticker");
        a.cfg.reloadable = BRIDGE_TRUE;
        a.cfg.hot_reload = strcmp(what, "maynot") ? BRIDGE_TRUE : BRIDGE_FALSE;
        load(b, &a);
        frames(b, 2);
        a = wasm("game:ticker@0.2.0", !strcmp(what, "reloadlayout") ? "ticker-wide"
                                    : !strcmp(what, "reloadfail") || !strcmp(what, "reloadexactfail")
                                      ? "ticker-fail"
                                    : !strcmp(what, "reloaddrop") ? "ticker-noextra" : "ticker-ten");
        a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
        /* the new instance states more ids than the old held ready: its own block of them is
         * as long as it states, and takes the old one's in */
        if (!strcmp(what, "reloadspawns")) a.cfg.spawns = 4000;
        if (!strcmp(what, "reloadmove")) {
            a = native("game:ticker@0.2.0", NTICKER1_PATH);
            a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
        }
        reload(b, &a);
        frames(b, 2);
        if (!strcmp(what, "reloaddrop")) {
            /* what the new instance no longer declares went as an unload would take it */
            a = wasm("game:retype@0.1.0", "retype-extra");
            a.cfg.spawns = BRIDGE_NONE;
            load(b, &a);
        }
        modules(b);
    } else if (!strcmp(what, "revive")) {       /* a stopped module, reloaded into one that works */
        a = wasm("game:ticker@0.1.0", "ticker-trip");
        a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
        load(b, &a);
        frames(b, 4);
        modules(b);
        a = wasm("game:ticker@0.2.0", "ticker-ten");
        a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
        reload(b, &a);
        frames(b, 2);
        modules(b);
    } else if (!strcmp(what, "notreloadable")) {
        /* the deployment allows what the author says cannot be done */
        a = wasm("game:twostep@0.1.0", "twostep");
        a.cfg.hot_reload = BRIDGE_TRUE;
        load(b, &a);
    } else if (!strcmp(what, "noreload")) {     /* one of the three exports is missing */
        a = wasm("test:noreload@0.1.0", "noreload");
        a.cfg.spawns = BRIDGE_NONE;
        load(b, &a);
    } else if (!strcmp(what, "reloadnothing")) {
        a = wasm("game:ticker@0.2.0", "ticker-ten");
        a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
        reload(b, &a);
    } else if (!strcmp(what, "ntimeout")) {     /* a timeout is a deadline, which native lacks */
        a = native("game:ticker@0.1.0", "no-such-library");
        a.cfg.shutdown_timeout = 100 * MS;
        load(b, &a);
    } else if (!strcmp(what, "nreload") || !strcmp(what, "nreloadfail")
               || !strcmp(what, "nreloadfailstop") || !strcmp(what, "nreloadfailunload")) {
        /* native: the old instance is shut down before the new library is opened */
        int essential = !strcmp(what, "nreloadfailstop");
        a = wasm("game:twostep@0.1.0", "twostep");
        load(b, &a);
        a = native("game:ticker@0.1.0", NTICKER1_PATH);
        a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
        if (essential) a.cfg.on_error = BRIDGE_FAIL;
        load(b, &a);
        frames(b, 2);
        a = native("game:ticker@0.2.0", !strcmp(what, "nreload") ? NTICKER2_PATH : NTICKER3_PATH);
        a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
        if (essential) a.cfg.on_error = BRIDGE_FAIL;
        reload(b, &a);
        frames(b, 2);
        modules(b);
        if (!strcmp(what, "nreloadfailunload")) {
            /* the verb that discards: a module with no code left can still be unloaded */
            unload(b, "game:ticker@0.1.0");
            modules(b);
        } else if (strcmp(what, "nreload")) {
            /* its rows were kept: an artifact that works picks up where the count was */
            a = native("game:ticker@0.3.0", NTICKER2_PATH);
            a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
            if (essential) a.cfg.on_error = BRIDGE_FAIL;
            reload(b, &a);
            frames(b, 2);
            modules(b);
        }
    } else if (!strcmp(what, "config")) {
        a = configured("game:settings@0.1.0");
        load(b, &a);
        frames(b, 1);
    } else if (!strcmp(what, "configkey") || !strcmp(what, "configkind") || !strcmp(what, "configlong")
               || !strcmp(what, "configrange") || !strcmp(what, "configenum")
               || !strcmp(what, "confignone") || !strcmp(what, "configmissing")
               || !strcmp(what, "configunset")
               || !strcmp(what, "configarena") || !strcmp(what, "configceiling")
               || !strcmp(what, "configwide")) {
        a = configured("game:settings@0.1.0");
        if (!strcmp(what, "configkey")) everything[0].key = "vsynk";
        if (!strcmp(what, "configkind")) { everything[2].kind = BRIDGE_STRING; everything[2].v.s = "fast"; }
        if (!strcmp(what, "configlong")) everything[4].v.s = "twenty characters...";
        if (!strcmp(what, "configrange")) everything[6].v.i = 300;
        if (!strcmp(what, "configenum")) everything[3].v.s = "Sometimes";
        /* the enum is an int8_t, as its author stated: a number past it is refused */
        if (!strcmp(what, "configwide")) { everything[3].kind = BRIDGE_INT; everything[3].v.i = 200; }
        if (!strcmp(what, "confignone")) a.cfg.config = BRIDGE_NO_CONFIG;
        if (!strcmp(what, "configunset")) a.cfg.config = nullptr;
        if (!strcmp(what, "configmissing")) a.cfg.config = "game:Nope";
        if (!strcmp(what, "configarena")) a.cfg.persistent = 8;
        /* the row its settings go on is an entity like any other, under the same ceiling */
        if (!strcmp(what, "configceiling")) a.cfg.spawns = BRIDGE_NONE;
        load(b, &a);
        modules(b);
    } else if (!strcmp(what, "configreload") || !strcmp(what, "configdrop")) {
        /* a reload hands over other settings, and the row keeps what it had: after init the
         * component is the state. With DROP the new instance declares nothing, and the row
         * goes with the component. */
        a = configured("game:settings@0.1.0");
        a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
        load(b, &a);
        frames(b, 1);
        everything[1].v.i = 640;
        a = configured("game:settings@0.2.0");
        a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
        if (!strcmp(what, "configdrop")) {
            a = wasm("game:settings@0.2.0", "settings-drop");
            a.cfg.hot_reload = a.cfg.reloadable = BRIDGE_TRUE;
            a.cfg.persistent = 64;
        }
        reload(b, &a);
        frames(b, 1);
        if (!strcmp(what, "configdrop")) {
            a = wasm("game:retype@0.1.0", "retype-settings");
            a.cfg.spawns = BRIDGE_NONE;
            load(b, &a);
        }
    } else if (!strcmp(what, "slots")) {
        /* [budget] modules: a record is taken at load and given back at unload */
        a = wasm("game:twostep@0.1.0", "twostep");
        load(b, &a);
        a = wasm("game:ticker@0.1.0", "ticker");
        load(b, &a);
        unload(b, "game:twostep@0.1.0");
        load(b, &a);
        modules(b);
    } else if (!strcmp(what, "churn")) {
        /* Modules whose types are enums and nested structs, loaded and unloaded many times.
         * Each unload frees tables, and each load makes new ones where the freed ones were:
         * a table id that outlived its table would be handed to the wrong one, and the
         * row-count check would say the index is out of step. */
        int i, refused = 0;
        quiet = 1;
        for (i = 0; i < 200; i ++) {
            a = wasm("game:nested@0.1.0", "nested");
            a.cfg.spawns = 8;
            refused += bridge_load(b, &a);
            frames(b, 1);
            (void)bridge_unload(b, "game:nested@0.1.0");
            a = configured("game:settings@0.1.0");
            refused += bridge_load(b, &a);
            frames(b, 1);
            (void)bridge_unload(b, "game:settings@0.1.0");
            a = wasm("game:keeper@0.1.0", "keeper-sweep");
            refused += bridge_load(b, &a);
            frames(b, 1);
            (void)bridge_unload(b, "game:keeper@0.1.0");
        }
        quiet = 0;
        printf("churned 200 times, %d loads refused, %u records of the index out of step\n", refused,
            out_of_step);
        modules(b);
    } else if (!strncmp(what, "arena", 5)) {
        /* The whole arena is one module's share, and it keeps a string there. Alone, its
         * unload gives the share back and the next module has it. With another module that
         * declares the same component the row stays, the string with it, and the share is
         * not given to anybody: the reader still says it, after the refusal as before. */
        a = wasm("game:namer@0.1.0", "namer");
        a.cfg.persistent = 64;
        load(b, &a);
        if (!strcmp(what, "arenakept")) {
            a = wasm("game:reader@0.1.0", "namer-reader");
            load(b, &a);
        }
        frames(b, 1);
        unload(b, "game:namer@0.1.0");
        frames(b, 1);
        a = wasm("game:ticker@0.1.0", "ticker");
        a.cfg.persistent = 64;
        load(b, &a);
        frames(b, 1);
    } else if (!strcmp(what, "configtwice")) {  /* one row, so one module */
        a = configured("game:settings@0.1.0");
        load(b, &a);
        a = configured("game:other@0.1.0");
        load(b, &a);
    } else if (!strcmp(what, "reseed")) {       /* unload, load: the text goes where it was */
        a = configured("game:settings@0.1.0");
        load(b, &a);
        frames(b, 1);
        unload(b, "game:settings@0.1.0");
        a = configured("game:settings@0.1.0");
        load(b, &a);
        frames(b, 1);
        printf("settings text %s\n", path_count == 2 && path_at[0] == path_at[1]
            ? "written to the same place" : "MOVED");
    }

    (void)bridge_destroy(b);
    printf("alive\n");
    return 0;
}
