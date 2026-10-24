/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_TESTS_BACKEND_INSIDE_H
    #define BRIDGE_TESTS_BACKEND_INSIDE_H

    #include <setjmp.h>
    #include <stddef.h>
    #include <string.h>

    #include "bridge_backend.h"
    #include "os.h"
    #include "test_module.h"

enum tests_entry : uint32_t {
    TESTS_DECLARE,
    TESTS_ADD_SYSTEM,
    TESTS_OBSERVE,
    TESTS_SPAWN,
    TESTS_DESTROY,
    TESTS_ATTACH,
    TESTS_DETACH,
    TESTS_ALLOC,
    TESTS_ENTRIES
};

static const char *const TESTS_ENTRY_NAMES[TESTS_ENTRIES] = {
    "declare",
    "add_system",
    "observe",
    "spawn",
    "destroy",
    "attach",
    "detach",
    "alloc"
};

struct backend {
    enum backend_capability capabilities;
    uint64_t reservation;
    uint64_t guard;
    uint64_t granule;
    uint32_t told;
};

struct backend_memory {
    void *ptr;
    uint64_t len;
};

struct backend_module {
    void *lib;
    void (*keep)(const test_api_t *);
    bool stray;
    bool crowd;
    bool insists;
    bool undefined;
};

struct backend_func {
    backend_instance_t *instance;
    void *fn;
    backend_func_t *next;
};

struct backend_instance {
    backend_module_t *m;
    test_api_t api;
    backend_fn_t fns[TESTS_ENTRIES];
    void *uds[TESTS_ENTRIES];
    uint64_t calling;
    int refused;
    int bounded;
    jmp_buf *out;
    const char *why;
    uint64_t work;
    uint64_t timeout;
    uint64_t memory_limit;
    uint64_t memory;
    backend_func_t *funcs;
};

typedef struct {
    enum backend_type params[3];
    uint32_t count;
    enum backend_type result;
} tests_shape_t;

static inline backend_instance_t *tests_get_instance(const test_api_t *api)
{
    char *base = (char *)api - offsetof(backend_instance_t, api);

    return ((backend_instance_t *)base);
}

static inline uint64_t tests_enter(const test_api_t *api,
    enum tests_entry entry, const uint64_t *args)
{
    backend_instance_t *i = tests_get_instance(api);
    uint64_t result = 0;

    if (!i->calling || (i->refused && !i->m->insists) || !i->fns[entry])
        return (0);
    if (i->calling != os_get_thread_id())
        return (0);
    if (!i->fns[entry](i->uds[entry], args, &result))
        return (result);
    i->refused = 1;
    return (0);
}

static inline void tests_take_memories(test_api_t *api,
    const backend_memory_import_t *memories, uint32_t count)
{
    uint32_t k = 0;

    while (k < count) {
        if (!strcmp(memories[k].field, BRIDGE_SHARED))
            api->shared_base = memories[k].memory->ptr;
        if (!strcmp(memories[k].field, BRIDGE_PRIVATE))
            api->private_base = memories[k].memory->ptr;
        k++;
    }
}

void tests_fill_api(test_api_t *api);

uint64_t tests_spawn(const test_api_t *api);
void tests_destroy(const test_api_t *api, uint64_t id);
void tests_attach(const test_api_t *api, uint64_t id, uint32_t slot,
    uint32_t ptr);
void tests_detach(const test_api_t *api, uint64_t id, uint32_t slot);
uint32_t tests_alloc(const test_api_t *api, uint32_t len);

void tests_trap(const test_api_t *api);
void tests_spend(const test_api_t *api, uint64_t units);
void tests_wait(const test_api_t *api, uint64_t nanos);
int tests_grow(const test_api_t *api, uint64_t bytes);

#endif
