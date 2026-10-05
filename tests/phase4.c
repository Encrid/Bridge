/* A host, as far as phase 4: two backends, and modules on both.
 *
 *     phase4 <case>
 *
 * Each case is one clause of docs/PLAN.md phase 4, checked by the records it emits. */
#include "bridge.h"
#include "bridge-abi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MS 1000000ll

static void sink(void *ud, const bridge_record *r)
{
    (void)ud;
    printf("record[%d] %s/%s: %.*s %u %u\n", r->level, r->identity ? r->identity : "-",
        r->system ? r->system : "-", (int)r->len, r->text, r->arg[0], r->arg[1]);
}

/* a native module as a deployment must state it: trusted, and every limit said to be none */
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
    a.cfg.spawns = 1024;
    a.cfg.persistent = BRIDGE_NONE;
    a.backend = "native";
    a.path = path;
    return a;
}

/* the reader of phase 3, on the wasm backend, with the limits that backend can keep */
static bridge_artifact wasm_reader(void)
{
    static const char *const grants[] = { "game:Position", "game:Marked" };
    bridge_artifact a = native("view:reader@0.1.0", NULL);
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
    a.cfg.trusted = false;
    a.cfg.fuel = 10000000;
    a.cfg.memory_limit = 16 << 20;
    a.cfg.init_timeout = 2000 * MS;
    a.cfg.read = grants;
    a.cfg.read_count = 2;
    a.cfg.spawns = BRIDGE_NONE;         /* it only reads */
    return a;
}

static void load(bridge *b, const bridge_artifact *a)
{
    printf("%s: %s\n", a->cfg.identity, bridge_load(b, a) ? "not loaded" : "loaded");
}

int main(int argc, char **argv)
{
    const char *what = argc > 1 ? argv[1] : "";
    bridge_backend backends[2] = {{ "wasm", BACKEND_PATH }, { "native", NATIVE_PATH }};
    const char *phases[2] = { "update", "present" };
    bridge_config cfg = { backends, 2, phases, 2, NULL, 0, 1, { 4096, 4096, 1 << 20, 8 } };
    bridge_setting cache = { "cache", BRIDGE_STRING, { .s = "somewhere" } };
    bridge_artifact a;
    bridge *b;
    int i;
    setvbuf(stdout, NULL, _IONBF, 0);

    if (!strcmp(what, "setting")) {             /* a key the backend does not know */
        backends[1].settings = &cache;
        backends[1].setting_count = 1;
    }

    b = bridge_create(&cfg, sink, NULL);
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
            bridge_phase(b, "update");
            bridge_phase(b, "present");
        }
    } else if (!strcmp(what, "untrusted")) {    /* refused before any library is looked for */
        a = native("game:mover@0.1.0", "no-such-library");
        a.cfg.trusted = false;
        load(b, &a);
    } else if (!strcmp(what, "limit")) {
        a = native("game:mover@0.1.0", "no-such-library");
        a.cfg.fuel = 1000;
        load(b, &a);
    } else if (!strcmp(what, "nolib")) {
        a = native("game:mover@0.1.0", "no-such-library");
        load(b, &a);
    } else if (!strcmp(what, "refuse")) {
        a = native("test:flood@0.1.0", NFLOOD_PATH);
        load(b, &a);
        bridge_phase(b, "update");
    }

    bridge_destroy(b);
    printf("alive\n");
    return 0;
}
