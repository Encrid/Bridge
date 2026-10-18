/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_BACKENDS_NATIVE_INSIDE_H
    #define BRIDGE_BACKENDS_NATIVE_INSIDE_H

    #include <stddef.h>

    #include "bridge_backend.h"
    #include "bridge_native.h"
    #include "os.h"

enum native_entry : uint32_t {
    NATIVE_DECLARE,
    NATIVE_ADD_SYSTEM,
    NATIVE_OBSERVE,
    NATIVE_SPAWN,
    NATIVE_DESTROY,
    NATIVE_ATTACH,
    NATIVE_DETACH,
    NATIVE_ALLOC,
    NATIVE_ENTRIES
};

static const char *const NATIVE_ENTRY_NAMES[NATIVE_ENTRIES] = {
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
    int unused;
};

struct backend_memory {
    void *ptr;
    uint64_t len;
};

struct backend_module {
    void *lib;
};

struct backend_func {
    backend_instance_t *instance;
    int (*fn)(const bridge_api_t *);
    backend_func_t *next;
};

struct backend_instance {
    backend_module_t *m;
    bridge_api_t api;
    backend_fn_t fns[NATIVE_ENTRIES];
    void *uds[NATIVE_ENTRIES];
    uint64_t calling;
    int refused;
    backend_func_t *funcs;
};

static inline uint64_t native_enter(const bridge_api_t *api,
    enum native_entry entry, const uint64_t *args)
{
    char *base = (char *)api - offsetof(backend_instance_t, api);
    backend_instance_t *i = (backend_instance_t *)base;
    uint64_t result = 0;

    if (i->refused || !i->fns[entry])
        return (0);
    if (i->calling != os_get_thread_id())
        return (0);
    if (!i->fns[entry](i->uds[entry], args, &result))
        return (result);
    i->refused = 1;
    return (0);
}

void native_fill_api(bridge_api_t *api);

uint64_t native_spawn(const bridge_api_t *api);
void native_destroy(const bridge_api_t *api, uint64_t id);
void native_attach(const bridge_api_t *api, uint64_t id, uint32_t slot,
    uint32_t ptr);
void native_detach(const bridge_api_t *api, uint64_t id, uint32_t slot);
uint32_t native_alloc(const bridge_api_t *api, uint32_t len);

#endif
