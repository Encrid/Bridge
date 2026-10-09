/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/binding/test_offsets_span.h -- a query's spans, read by offsets.
**
** query.md, *The interface is a query*
*/

#ifndef TEST_OFFSETS_SPAN_H
    #define TEST_OFFSETS_SPAN_H

    #include "test_offsets_acc.h"

/*
** bridge_count_spans
**
** Answers how many spans a query has at this barrier.
**
** query.md, *The interface is a query*
*/
static inline uint32_t bridge_count_spans(uint32_t query)
{
    uint32_t at = BRIDGE_HEADER(queries);

    return (acc_own_32(at + query * sizeof(bridge_query_t)));
}

/*
** bridge_get_span
**
** Answers span `i` of a query that declared `terms` terms.
**
** query.md, *The interface is a query*
*/
static inline uint32_t bridge_get_span(uint32_t query, uint32_t i,
    uint32_t terms)
{
    uint32_t at = BRIDGE_HEADER(queries) + query * sizeof(bridge_query_t);

    return (acc_own_32(at + 4u) + i * BRIDGE_SPAN_SIZE(terms));
}

/*
** bridge_count_rows
**
** Answers how many rows a span has.
**
** query.md, *The interface is a query*
*/
static inline uint32_t bridge_count_rows(uint32_t span)
{
    return (acc_own_32(span));
}

/*
** bridge_get_entities
**
** Answers where the ids of a span's rows are.
**
** query.md, *The interface is a query*
*/
static inline uint32_t bridge_get_entities(uint32_t span)
{
    return (acc_own_32(span + 4u));
}

/*
** bridge_get_base
**
** Answers where a term's column begins in a span. Zero: the term is absent
** in this table.
**
** query.md, *The interface is a query*
*/
static inline uint32_t bridge_get_base(uint32_t span, uint32_t term)
{
    return (acc_own_32(span + BRIDGE_SPAN_BASE + 4u * term));
}

#endif
