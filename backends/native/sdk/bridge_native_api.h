/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** backends/native/sdk/bridge_native_api.h -- the table the native backend
** hands a native module, and the scratch arena, where a module puts what an
** entry of the ABI is handed.
**
** What is compiled with this file is a work based on Bridge and is under
** the same licence. Other terms: docs/licence.md.
**
** native.md, *What the backend provides*
*/

#ifndef BRIDGE_NATIVE_API_H
    #define BRIDGE_NATIVE_API_H

    #include <stddef.h>
    #include <string.h>

    #include "bridge_abi.h"
    #include "bridge_schema.h"

    #ifdef _WIN32
        #define BRIDGE_NATIVE_EXPORT [[gnu::dllexport]]
    #else
        #define BRIDGE_NATIVE_EXPORT [[gnu::visibility("default")]]
    #endif

/*
** What a native module exports: a function of this name, handed the table
** as `api`. A system called "move" is the export bridge_move.
**
** native.md, *What the backend provides*
*/
    #define BRIDGE_NATIVE(name) \
        BRIDGE_NATIVE_EXPORT int bridge_##name( \
            [[maybe_unused]] const bridge_api_t *api)

/*
** A struct of type T with these members, put in the scratch arena: where it
** is.
**
** memory.md, *Where the regions live*
*/
    #define BRIDGE_NATIVE_STATE(api, T, ...) \
        bridge_native_put(api, &(T){ __VA_ARGS__ }, sizeof(T))

/*
** What the native backend hands a native module: the eight entries of the
** ABI and where the two memories begin. It is passed to every export, and
** every entry takes it first, so nothing is found by who happens to be
** running. Every pointer argument of an entry is still an offset into the
** scratch arena: that is Bridge's, and the same on every backend.
**
** An entry may be called only from inside a call the backend made, on the
** thread it made it on: init, a system, an observer. The backend checks it.
**
**     shared_base   the shared memory. It does not move
**     private_base  this module's private memory
**
** abi.md, *The ABI*
** native.md, *The invariant*
*/
typedef struct bridge_api bridge_api_t;

struct bridge_api {
    uint32_t (*declare)(const bridge_api_t *api, uint32_t at);
    uint32_t (*add_system)(const bridge_api_t *api, uint32_t at);
    uint32_t (*observe)(const bridge_api_t *api, uint32_t at);
    uint64_t (*spawn)(const bridge_api_t *api);
    void (*destroy)(const bridge_api_t *api, uint64_t id);
    void (*attach)(const bridge_api_t *api, uint64_t id, uint32_t slot,
        uint32_t ptr);
    void (*detach)(const bridge_api_t *api, uint64_t id, uint32_t slot);
    uint32_t (*alloc)(const bridge_api_t *api, uint32_t len);
    uint8_t *shared_base;
    uint8_t *private_base;
};

/*
** bridge_native_get_header
**
** Answers the header of the module's private memory.
**
** memory.md, *Where the regions live*
*/
static inline bridge_header_t *bridge_native_get_header(const bridge_api_t *api)
{
    return ((bridge_header_t *)api->private_base);
}

/*
** bridge_native_put
**
** Copies `len` bytes into the scratch arena, and answers where they are, or
** 0 if they do not fit, which every entry refuses.
**
** memory.md, *Where the regions live*
*/
static inline uint32_t bridge_native_put(const bridge_api_t *api,
    const void *bytes, uint32_t len)
{
    bridge_header_t *h = bridge_native_get_header(api);
    uint32_t used = (h->scratch_used + 7u) & ~7u;

    if (used > h->scratch_len || len > h->scratch_len - used)
        return (0);
    memcpy(api->private_base + h->scratch + used, bytes, len);
    h->scratch_used = used + len;
    return (h->scratch + used);
}

/*
** bridge_native_lend
**
** Copies `len` bytes above what the scratch arena has in use, and does not
** count them as used: where they are, or 0 if they do not fit. For a value
** an entry copies during the call, as attach does, so that a module's
** scratch holds one such value at a time and not a phase's worth.
**
** memory.md, *Where the regions live*
*/
static inline uint32_t bridge_native_lend(const bridge_api_t *api,
    const void *bytes, uint32_t len)
{
    const bridge_header_t *h = bridge_native_get_header(api);
    uint32_t used = (h->scratch_used + 7u) & ~7u;

    if (used > h->scratch_len || len > h->scratch_len - used)
        return (0);
    memcpy(api->private_base + h->scratch + used, bytes, len);
    return (h->scratch + used);
}

#endif
