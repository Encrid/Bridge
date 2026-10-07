/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** backends/native/native_inside.h -- the native backend: the seventeen
** entries of bridge_backend.h, over a shared library. What its files share.
**
** native.md, *The invariant*
** capabilities.md, *Capabilities, so the swap is honest*
*/

#ifndef BRIDGE_BACKENDS_NATIVE_INSIDE_H
    #define BRIDGE_BACKENDS_NATIVE_INSIDE_H

    #include <stddef.h>

    #include "bridge_backend.h"
    #include "bridge_native.h"
    #include "os.h"

/*
** The entries of the ABI, in the order of their names.
*/
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

/*
** An instance.
**
**     api           the table the module is handed
**     calling       the thread a call into it is in progress on, or 0
**     refused       an entry refused during the call in progress
*/
struct backend_instance {
    backend_module_t *m;
    bridge_api_t api;
    backend_fn_t fns[NATIVE_ENTRIES];
    void *uds[NATIVE_ENTRIES];
    uint64_t calling;
    int refused;
    backend_func_t *funcs;
};

/*
** native_enter
**
** An entry of the ABI, reached from a module. The table a module was handed
** is part of its instance, so an entry is told whose it is by the table it
** is given. Native code runs only inside a call this backend made, and that
** is checked here: an entry reached from any other thread, a foreign
** thread's callback say, or outside a call, answers zero.
**
** A refusal cannot unwind native code, so the entry answers zero and the
** call returns: Bridge-ASM700 fails the invocation then. Until it does
** return, every entry answers zero and reaches nothing, which is as near to
** stopped as native code can be put.
**
** native.md, *The invariant*
** signatures.md, *A call takes nothing and answers a status*
*/
static inline uint64_t native_enter(const bridge_api_t *api,
    enum native_entry entry, const uint64_t *args)
{
    char *base = (char *)api - offsetof(backend_instance_t, api);
    backend_instance_t *i = (backend_instance_t *)base;
    uint64_t result = 0;

    if (!i->calling || i->refused || !i->fns[entry])
        return (0);
    if (i->calling != os_get_thread_id())
        return (0);
    if (!i->fns[entry](i->uds[entry], args, &result))
        return (result);
    i->refused = 1;
    return (0);
}

/*
** native_fill_api
**
** Writes the eight entries into the table a module is handed.
*/
void native_fill_api(bridge_api_t *api);

/*
** The entries a module calls in a phase, as the table has them.
*/
uint64_t native_spawn(const bridge_api_t *api);
void native_destroy(const bridge_api_t *api, uint64_t id);
void native_attach(const bridge_api_t *api, uint64_t id, uint32_t slot,
    uint32_t ptr);
void native_detach(const bridge_api_t *api, uint64_t id, uint32_t slot);
uint32_t native_alloc(const bridge_api_t *api, uint32_t len);

#endif
