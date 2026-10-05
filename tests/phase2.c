/* A host, as far as phase 2: it creates a bridge, loads one module, and prints every record.
 *
 *     phase2 <case>
 *
 * Each case is one claim of docs/PLAN.md phase 2. build.py checks a refusal by the record it
 * emits, not by a return code alone, and every case ends by printing "alive". */
#include "bridge.h"
#include "bridge-abi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constexpr int64_t MS = 1'000'000;

static void sink([[maybe_unused]] void *ud, const bridge_record *r)
{
    printf("record[%d]: %.*s\n", r->level, (int)r->len, r->text);
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
static bridge_artifact artifact(const char *name)
{
    static char identity[64];
    bridge_artifact a = {};
    snprintf(identity, sizeof identity, "test:%s@0.1.0", name);
    a.cfg.identity = identity;
    a.cfg.abi = BRIDGE_ABI;
    a.cfg.trusted = false;
    a.cfg.on_error = BRIDGE_SKIP;
    a.cfg.max_system_failures = 3;
    a.cfg.fuel = 1'000'000;
    a.cfg.memory_limit = 1 << 20;
    a.cfg.init_timeout = 200 * MS;
    a.cfg.reload_timeout = 200 * MS;
    a.cfg.shutdown_timeout = 200 * MS;
    a.cfg.scratch = 4096;
    a.cfg.log_ring = 240;
    a.cfg.spans = 4096;
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.persistent = BRIDGE_NONE;
    a.cfg.config = BRIDGE_NO_CONFIG;
    a.backend = "wasm";
    a.bytes = read_file(name, &a.len);
    return a;
}

static int load(bridge *b, const bridge_artifact *a)
{
    int r = bridge_load(b, a);
    printf("%s: %s\n", a->cfg.identity, r ? "not loaded" : "loaded");
    return r;
}

int main(int argc, char **argv)
{
    const char *what = argc > 1 ? argv[1] : "";
    bridge_backend backends[1] = {{ "wasm", BACKEND_PATH }};
    const char *phases[1] = { "update" };
    bridge_config cfg = { backends, 1, phases, 1, nullptr, 0, 1, { 4096, 4096, 1 << 20, 8, 64 } };
    bridge_artifact a;
    bridge *b;
    setvbuf(stdout, nullptr, _IONBF, 0);

    if (!strcmp(what, "nosink")) {
        printf("%s\n", bridge_create(&cfg, nullptr, nullptr) ? "created" : "refused: no sink");
        printf("alive\n");
        return 0;
    }
    if (!strcmp(what, "nolib")) backends[0].library = "no-such-backend-library";

    b = bridge_create(&cfg, sink, nullptr);
    if (!b) {
        printf("bridge_create refused\nalive\n");
        return 0;
    }

    if (!strcmp(what, "abi")) {
        a = artifact("ok");
        a.cfg.abi = BRIDGE_ABI + 1;
        load(b, &a);
    } else if (!strcmp(what, "nobackend")) {
        a = artifact("ok");
        a.backend = "native";
        load(b, &a);
    } else if (!strcmp(what, "unset")) {
        a = artifact("ok");
        a.cfg.memory_limit = 0;
        load(b, &a);
    } else if (!strcmp(what, "nolimit")) {      /* "none" is a value, and it loads */
        a = artifact("ok");
        a.cfg.fuel = BRIDGE_NO_LIMIT;
        a.cfg.memory_limit = BRIDGE_NO_LIMIT;
        a.cfg.init_timeout = BRIDGE_NO_LIMIT;
        a.cfg.reload_timeout = BRIDGE_NO_LIMIT;
        a.cfg.shutdown_timeout = BRIDGE_NO_LIMIT;
        load(b, &a);
    } else if (!strcmp(what, "stop")) {         /* fail stops the bridge; skip does not */
        a = artifact("oob");
        load(b, &a);
        a = artifact("ok");
        load(b, &a);
        a = artifact("oob");
        a.cfg.on_error = BRIDGE_FAIL;
        load(b, &a);
        a = artifact("ok");
        load(b, &a);
    } else {
        a = artifact(what);
        load(b, &a);
    }

    (void)bridge_destroy(b);
    printf("alive\n");
    return 0;
}
