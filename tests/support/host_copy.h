/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_HOST_COPY_H
    #define TEST_HOST_COPY_H

    #include <stddef.h>

    #include "bridge.h"

constexpr size_t HOST_COPY_ROOM = 8192;

typedef struct {
    unsigned char *block;
    size_t low;
    size_t high;
} host_copy_t;

[[nodiscard]] const bridge_artifact_t *host_copy(host_copy_t *c,
    const bridge_artifact_t *a, uint32_t count);

[[nodiscard]] void *host_take(host_copy_t *c, const void *from, size_t size);

[[nodiscard]] const char *host_take_text(host_copy_t *c, const char *text);

void host_spoil(void **spoiled, host_copy_t *c);

void host_free_spoiled(void **spoiled);

#endif
