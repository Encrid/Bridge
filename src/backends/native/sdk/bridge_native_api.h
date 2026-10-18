/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

    #define BRIDGE_NATIVE(name) \
        BRIDGE_NATIVE_EXPORT int bridge_##name( \
            [[maybe_unused]] const bridge_api_t *api)

    #define BRIDGE_NATIVE_STATE(api, T, ...) \
        bridge_native_put(api, &(T){ __VA_ARGS__ }, sizeof(T))

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

static inline bridge_header_t *bridge_native_get_header(const bridge_api_t *api)
{
    return ((bridge_header_t *)api->private_base);
}

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
