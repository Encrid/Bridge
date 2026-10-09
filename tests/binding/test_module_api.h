/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/binding/test_module_api.h -- the table a module of the tests is
** handed, what it has for the backend to call, and the scratch arena
** written to.
**
** abi.md, *The ABI*
*/

#ifndef TEST_MODULE_API_H
    #define TEST_MODULE_API_H

    #include <stddef.h>
    #include <string.h>

    #include "bridge_abi.h"
    #include "bridge_schema.h"

    #ifdef _WIN32
        #define TEST_EXPORT [[gnu::dllexport]]
    #else
        #define TEST_EXPORT [[gnu::visibility("default")]]
    #endif

/*
** What a module of the tests exports for the tests' backend to call: a
** function of this name, handed the table as `api`. A system called "move"
** is the export module_move, which a module that calls one of its own
** writes as TEST_OWN(move).
**
** abi.md, *The ABI*
*/
    #define TEST_OWN(name) module_##name
    #define TEST(name) \
        TEST_EXPORT int module_##name( \
            [[maybe_unused]] const test_api_t *api)

/*
** A struct of type T with these members, put in the scratch arena: where it
** is.
**
** memory.md, *Where the regions live*
*/
    #define TEST_STATE(api, T, ...) \
        test_put(api, &(T){ __VA_ARGS__ }, sizeof(T))

/*
** What the tests' backend hands a module: the eight entries of the ABI,
** where the two memories begin, and four moves that are the tests' own. It
** is passed to every function of a module, and every entry takes it first,
** so nothing is found by who happens to be running. Every pointer argument
** of an entry is still an offset into the scratch arena: that is Bridge's,
** and the same on every backend.
**
** An entry may be called only from inside a call the backend made, on the
** thread it made it on: init, a system, an observer. The backend checks it.
**
**     shared_base   the shared memory. It does not move
**     private_base  this module's private memory
**     trap          does not return: the call that reached it is ended, as
**                   a module that faults is where a backend can catch it
**     spend         counts units of work against the limit Bridge set
**                   for this invocation, and does not return past it
**     wait          counts nanoseconds against the timeout Bridge set for
**                   this call, and does not return past it. No time passes
**     grow          asks for bytes of memory of the module's own: 1, or 0
**                   past the limit Bridge set
**
** abi.md, *The ABI*
** signatures.md, *A call takes nothing and answers a status*
*/
typedef struct test_api test_api_t;

struct test_api {
    uint32_t (*declare)(const test_api_t *api, uint32_t at);
    uint32_t (*add_system)(const test_api_t *api, uint32_t at);
    uint32_t (*observe)(const test_api_t *api, uint32_t at);
    uint64_t (*spawn)(const test_api_t *api);
    void (*destroy)(const test_api_t *api, uint64_t id);
    void (*attach)(const test_api_t *api, uint64_t id, uint32_t slot,
        uint32_t ptr);
    void (*detach)(const test_api_t *api, uint64_t id, uint32_t slot);
    uint32_t (*alloc)(const test_api_t *api, uint32_t len);
    uint8_t *shared_base;
    uint8_t *private_base;
    void (*trap)(const test_api_t *api);
    void (*spend)(const test_api_t *api, uint64_t units);
    void (*wait)(const test_api_t *api, uint64_t nanos);
    int (*grow)(const test_api_t *api, uint64_t bytes);
};

/*
** test_get_header
**
** Answers the header of the module's private memory.
**
** memory.md, *Where the regions live*
*/
static inline bridge_header_t *test_get_header(const test_api_t *api)
{
    return ((bridge_header_t *)api->private_base);
}

/*
** test_put
**
** Copies `len` bytes into the scratch arena, and answers where they are, or
** 0 if they do not fit, which every entry refuses.
**
** memory.md, *Where the regions live*
*/
static inline uint32_t test_put(const test_api_t *api,
    const void *bytes, uint32_t len)
{
    bridge_header_t *h = test_get_header(api);
    uint32_t used = (h->scratch_used + 7u) & ~7u;

    if (used > h->scratch_len || len > h->scratch_len - used)
        return (0);
    memcpy(api->private_base + h->scratch + used, bytes, len);
    h->scratch_used = used + len;
    return (h->scratch + used);
}

/*
** test_lend
**
** Copies `len` bytes above what the scratch arena has in use, and does not
** count them as used: where they are, or 0 if they do not fit. For a value
** an entry copies during the call, as attach does, so that a module's
** scratch holds one such value at a time and not a phase's worth.
**
** memory.md, *Where the regions live*
*/
static inline uint32_t test_lend(const test_api_t *api,
    const void *bytes, uint32_t len)
{
    const bridge_header_t *h = test_get_header(api);
    uint32_t used = (h->scratch_used + 7u) & ~7u;

    if (used > h->scratch_len || len > h->scratch_len - used)
        return (0);
    memcpy(api->private_base + h->scratch + used, bytes, len);
    return (h->scratch + used);
}

#endif
