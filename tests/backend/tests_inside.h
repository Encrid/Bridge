/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/backend/tests_inside.h -- the tests' backend: the entries of
** bridge_backend.h, over a module's library, with the limits kept by
** counting. What its files share. It is in no package, and no deployment
** is handed it.
**
** backend.md, *What a backend is asked for*
** capabilities.md, *Capabilities, so the swap is honest*
*/

#ifndef BRIDGE_TESTS_BACKEND_INSIDE_H
    #define BRIDGE_TESTS_BACKEND_INSIDE_H

    #include <setjmp.h>
    #include <stddef.h>
    #include <string.h>

    #include "bridge_backend.h"
    #include "os.h"
    #include "test_module.h"

/*
** The entries of the ABI, in the order of their names.
*/
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

/*
** The backend: what it was told it keeps, and the shape it was told to ask
** of a block.
*/
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

/*
** A module.
**
**     keep          what a module written by offsets exports, and is called
**                   with before each call: such a module's functions take
**                   nothing, and are exported by their own names. nullptr
**                   for a module whose functions take the table
**     stray         it exports test_import_stray: it is said to import a
**                   thing nobody answers, and no instance of it is made
*/
struct backend_module {
    void *lib;
    void (*keep)(const test_api_t *);
    bool stray;
};

struct backend_func {
    backend_instance_t *instance;
    void *fn;
    backend_func_t *next;
};

/*
** An instance.
**
**     api           the table the module is handed
**     calling       the thread a call into it is in progress on, or 0
**     refused       an entry refused during the call in progress
**     out           where the call in progress is ended from, and `why`
**     work          what is left of the work limit, if `bounded`
**     timeout       the timeout of the next call, or 0
**     memory_limit  the most memory of its own it may have, or 0
*/
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

/*
** What an entry takes and answers: its parameters, how many, and whether
** it answers 32 bits, 64, or nothing.
*/
typedef struct {
    enum backend_type params[3];
    uint32_t count;
    enum backend_type result;
} tests_shape_t;

/*
** tests_get_instance
**
** Answers whose table this is: the table a module was handed is part of
** its instance.
*/
static inline backend_instance_t *tests_get_instance(const test_api_t *api)
{
    char *base = (char *)api - offsetof(backend_instance_t, api);

    return ((backend_instance_t *)base);
}

/*
** tests_enter
**
** An entry of the ABI, reached from a module. Code of a module runs only
** inside a call this backend made, and that is checked here: an entry
** reached from any other thread, or outside a call, answers zero. After a
** refusal every entry answers zero until the call returns.
**
** signatures.md, *A call takes nothing and answers a status*
*/
static inline uint64_t tests_enter(const test_api_t *api,
    enum tests_entry entry, const uint64_t *args)
{
    backend_instance_t *i = tests_get_instance(api);
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
** tests_take_memories
**
** Keeps where the two memories an instance was handed begin.
*/
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

/*
** tests_fill_api
**
** Writes the eight entries and the four moves into the table a module is
** handed.
*/
void tests_fill_api(test_api_t *api);

/*
** The entries a module calls in a phase, as the table has them.
*/
uint64_t tests_spawn(const test_api_t *api);
void tests_destroy(const test_api_t *api, uint64_t id);
void tests_attach(const test_api_t *api, uint64_t id, uint32_t slot,
    uint32_t ptr);
void tests_detach(const test_api_t *api, uint64_t id, uint32_t slot);
uint32_t tests_alloc(const test_api_t *api, uint32_t len);

/*
** The four moves, as the table has them.
*/
void tests_trap(const test_api_t *api);
void tests_spend(const test_api_t *api, uint64_t units);
void tests_wait(const test_api_t *api, uint64_t nanos);
int tests_grow(const test_api_t *api, uint64_t bytes);

#endif
