/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "storage_test.h"

constexpr uint32_t WIDE = 70'000;
constexpr uint32_t WIDE_SCRATCH = 256;
constexpr uint32_t WIDE_RING = 1024;

static void hear_wide([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    printf("barriers %u place %u system %s\n", r->barriers, r->place,
        r->system ? r->system : "-");
}

static void write_line(uint8_t *priv)
{
    bridge_header_t *h = (bridge_header_t *)priv;
    bridge_log_record_t line = { .barriers = 7, .place = WIDE,
        .system = WIDE, .level = BRIDGE_LOG_INFO, .text = WIDE_SCRATCH,
        .len = 4 };

    memcpy(priv + WIDE_SCRATCH, "wide", 4);
    memcpy(priv + WIDE_RING, &line, sizeof(line));
    h->ring_head = 1;
}

enum storage_test_end storage_test_drain_wide_record(
    [[maybe_unused]] storage_test_t *t)
{
    alignas(8) static uint8_t priv[2048];
    char name[] = "far";
    bridge_system_t far = { .name = name };
    bridge_system_t **systems = calloc(WIDE + 1, sizeof(*systems));
    bridge_t b = { .sink = hear_wide };
    module_t m = { .priv = priv, .scratch = WIDE_SCRATCH,
        .scratch_len = WIDE_SCRATCH, .ring = WIDE_RING, .ring_len = 1,
        .systems = systems, .system_count = WIDE + 1 };

    if (!systems)
        return (STORAGE_TEST_FAILED);
    systems[WIDE] = &far;
    write_line(priv);
    block_drain(&b, &m);
    free(systems);
    return (STORAGE_TEST_DONE);
}
