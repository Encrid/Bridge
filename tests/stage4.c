/* A host, as far as stage 4: two backends, and modules on both.
 *
 *     stage4 <case>
 *
 * Each case is one clause of docs/PLAN.md stage 4, checked by the records it emits. */
#include "bridge.h"
#include "bridge_abi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

/* whether a library is in this process, asked of the operating system and of nothing of ours */
static bool loaded(const char *path)
{
#ifdef _WIN32
    char full[MAX_PATH];
    DWORD len = GetFullPathNameA(path, sizeof full, full, nullptr);
    return len && len < sizeof full && GetModuleHandleA(full);
#else
    void *lib = dlopen(path, RTLD_NOLOAD | RTLD_NOW);
    if (lib) dlclose(lib);
    return lib != nullptr;
#endif
}

constexpr int64_t MS = 1'000'000;

static void sink([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    printf("record[%d] %s/%s: %.*s %u %u\n", r->level, r->identity ? r->identity : "-",
        r->system ? r->system : "-", (int)r->len, r->text, r->arg[0], r->arg[1]);
}

/* a native module as a deployment must state it: trusted, and every limit said to be none */
static bridge_artifact_t native(const char *identity, const char *path)
{
    bridge_artifact_t a = {};
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
    a.cfg.spawns = 1024;
    a.cfg.persistent = BRIDGE_NONE;
    a.cfg.changes = 1 << 20;
    a.cfg.loss_count = BRIDGE_NONE;
    a.cfg.config = BRIDGE_NO_CONFIG;
    a.backend = "native";
    a.path = path;
    return a;
}

/* the reader of stage 3, on the wasm backend, with the limits that backend can keep */
static bridge_artifact_t wasm_reader()
{
    static const char *const grants[] = { "game:Position", "game:Marked" };
    bridge_artifact_t a = native("view:reader@0.1.0", nullptr);
    FILE *f = fopen(MODULE_DIR "/reader.wasm", "rb");
    void *data;
    if (!f) { printf("cannot read reader.wasm\n"); exit(2); }
    fseek(f, 0, SEEK_END);
    a.len = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(a.len);
    if (fread(data, 1, a.len, f) != a.len) { printf("cannot read reader.wasm\n"); exit(2); }
    fclose(f);
    a.bytes = data;
    a.backend = "wasm";
    a.cfg.trusted = BRIDGE_FALSE;
    a.cfg.work_limit = 10'000'000;
    a.cfg.memory_limit = 16 << 20;
    a.cfg.init_timeout = 2000 * MS;
    a.cfg.reload_timeout = 2000 * MS;
    a.cfg.shutdown_timeout = 2000 * MS;
    a.cfg.read = grants;
    a.cfg.read_count = 2;
    a.cfg.spawns = BRIDGE_NONE;         /* it only reads */
    return a;
}

static void load(bridge_t *b, const bridge_artifact_t *a)
{
    printf("%s: %s\n", a->cfg.identity, bridge_load(b, a) ? "not loaded" : "loaded");
}

int main(int argc, char **argv)
{
    const char *what = argc > 1 ? argv[1] : "";
    bridge_backend_t backends[2] = {{ "wasm", BACKEND_PATH }, { "native", NATIVE_PATH }};
    const char *phases[2] = { "update", "present" };
    bridge_config_t cfg = { .abi = BRIDGE_HOST_ABI, .backends = backends, .backend_count = 2, .phases = phases,
        .phase_count = 2, .threads = 1, .budget =
        { .entities = 4096, .tables = 64, .types = 64, .systems = 128, .terms = 8, .modules = 8,
          .persistent = 4096, .columns = 1 << 20,
          .sparse_sets = BRIDGE_NONE, .direct_indexed = BRIDGE_NONE } };
    bridge_setting_t cache = { .key = "cache", .kind = BRIDGE_STRING, .v = { .s = "somewhere" } };
    bridge_artifact_t a;
    bridge_t *b;
    int i;
    setvbuf(stdout, nullptr, _IONBF, 0);

    /* a backend library that says it is of another backend seam */
    if (!strcmp(what, "backendabi")) backends[1].library = NATIVE_OTHER_PATH;
    if (!strcmp(what, "setting")) {             /* a key the backend does not know */
        backends[1].settings = &cache;
        backends[1].setting_count = 1;
    }

    if (!strcmp(what, "libraries")) {
        /* What a destroyed bridge leaves in the process: the library of a backend that does
         * not say it can be unloaded, and not the library of one that does. The wasm
         * backend's brought wasmtime in, whose own threads do not survive being unloaded.
         * docs/seam/backend/capabilities.md.
         *
         * Bridge-ASM700 closes the native backend's library, and every system this is built
         * for then takes it out of the process. macOS did not while that backend had a
         * thread-local variable: it keeps such an image mapped. */
        b = bridge_create(&cfg, sink, nullptr);
        if (!b || bridge_destroy(b)) {
            printf("FAIL: the bridge was not created and destroyed\n");
            return 1;
        }
        printf("the wasm backend's library is %s, the native backend's is %s\n",
            loaded(BACKEND_PATH) ? "still loaded" : "GONE",
            loaded(NATIVE_PATH) ? "STILL LOADED" : "gone");
        printf("alive\n");
        return 0;
    }

    b = bridge_create(&cfg, sink, nullptr);
    if (!b) {
        printf("bridge_create refused\nalive\n");
        return 0;
    }

    if (!strcmp(what, "raw") || !strcmp(what, "mixed")) {
        a = native("game:mover@0.1.0", NMOVER_PATH);
        load(b, &a);
        if (!strcmp(what, "mixed")) {           /* a wasm system in the phase of a native one */
            a = wasm_reader();
            load(b, &a);
        }
        for (i = 0; i < 2; i ++) {
            (void)bridge_phase(b, "update");
            (void)bridge_phase(b, "present");
        }
    } else if (!strcmp(what, "untrusted")) {    /* refused before any library is looked for */
        a = native("game:mover@0.1.0", "no-such-library");
        a.cfg.trusted = BRIDGE_FALSE;
        load(b, &a);
    } else if (!strcmp(what, "limit")) {
        a = native("game:mover@0.1.0", "no-such-library");
        a.cfg.work_limit = 1000;
        load(b, &a);
    } else if (!strcmp(what, "nopath")) {       /* a native module, and nowhere to find it */
        a = native("game:mover@0.1.0", nullptr);
        load(b, &a);
    } else if (!strcmp(what, "nobytes")) {      /* a wasm module, and nothing to compile */
        a = wasm_reader();
        a.bytes = nullptr;
        a.len = 0;
        load(b, &a);
    } else if (!strcmp(what, "nolib")) {
        a = native("game:mover@0.1.0", "no-such-library");
        load(b, &a);
    } else if (!strcmp(what, "refuse")) {
        a = native("test:flood@0.1.0", NFLOOD_PATH);
        load(b, &a);
        (void)bridge_phase(b, "update");
    }

    (void)bridge_destroy(b);
    printf("alive\n");
    return 0;
}
