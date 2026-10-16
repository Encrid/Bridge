/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

    #define TEST_OWN(name) module_##name
    #define TEST(name) \
        TEST_EXPORT int module_##name( \
            [[maybe_unused]] const test_api_t *api)

    #define TEST_STATE(api, T, ...) \
        test_put(api, &(T){ __VA_ARGS__ }, sizeof(T))

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

static inline bridge_header_t *test_get_header(const test_api_t *api)
{
    return ((bridge_header_t *)api->private_base);
}

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
