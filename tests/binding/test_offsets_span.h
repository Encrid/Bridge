/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_OFFSETS_SPAN_H
    #define TEST_OFFSETS_SPAN_H

    #include "test_offsets_acc.h"

static inline uint32_t bridge_get_span_count(uint32_t query)
{
    uint32_t at = BRIDGE_HEADER(queries);

    return (acc_get_own_32(at + query * sizeof(bridge_query_t)));
}

static inline uint32_t bridge_get_span(uint32_t query, uint32_t i)
{
    uint32_t at = BRIDGE_HEADER(queries) + query * sizeof(bridge_query_t);

    return (acc_get_own_32(at + 4u) + i * BRIDGE_SPAN_SIZE);
}

static inline uint32_t bridge_get_span_table(uint32_t span)
{
    return (acc_get_own_32(span));
}

static inline uint32_t bridge_get_rows(uint32_t span)
{
    return (acc_get_rows(bridge_get_span_table(span)));
}

#endif
