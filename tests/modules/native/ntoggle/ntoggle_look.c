/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/ntoggle/ntoggle_look.c -- the systems of ntoggle
** that read the rows that are on: found a word of bits at a time, and by a
** test at every row.
*/

#include "ntoggle.h"
#include "native/native_measured.h"

uint32_t ntoggle_reading;

/*
** see_word
**
** Counts the rows whose bits are set in `word`, which is word `w` of the
** bits of `column`, and adds their values. Sixty-four rows that are all on
** are read as a column is.
*/
static void see_word(const t_t *column, uint64_t word, uint32_t w,
    seen_t *seen)
{
    uint32_t r = w * 64;

    if (word == UINT64_MAX) {
        while (r < w * 64 + 64) {
            seen->total += column[r].v;
            r++;
        }
        seen->on += 64;
        return;
    }
    while (word) {
        seen->total += column[w * 64 + (uint32_t)__builtin_ctzll(word)].v;
        seen->on++;
        word &= word - 1;
    }
}

/*
** see_words
**
** Counts the rows of a span that are on, a word of bits at a time. The last
** word has bits past the last row, which are nobody's, and are set aside.
*/
static void see_words(const bridge_api_t *api, const uint32_t *span,
    seen_t *seen)
{
    const t_t *column = (const t_t *)(api->shared_base + span[2]);
    const uint64_t *bits = bridge_native_get_bits(api, span[2]);
    const uint32_t count = span[0];
    const uint32_t whole = count / 64;
    uint64_t word;
    uint32_t w = 0;

    while (w <= whole) {
        word = bits[w];
        if (w == whole)
            word = count % 64 ? word & (((uint64_t)1 << (count % 64)) - 1) : 0;
        see_word(column, word, w, seen);
        w++;
    }
}

/*
** look
**
** Says the rows that are on, found a word of bits at a time: the rows, how
** many of them are on, the sum of those, and the spans.
*/
NATIVE_MEASURED BRIDGE_NATIVE(look)
{
    const bridge_query_t *q = bridge_native_get_query(api, ntoggle_reading);
    const uint32_t *span;
    seen_t seen = {};
    uint32_t rows = 0;
    uint32_t i = 0;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 1);
        see_words(api, span, &seen);
        rows += span[0];
        i++;
    }
    say(api, "by words: rows, and of them on", rows, seen.on);
    say(api, "by words: the sum of those on, and spans", seen.total, q->spans);
    return (0);
}

/*
** see_rows
**
** Counts the rows that are on by a test of the bit of every row, and adds
** their values.
*/
static void see_rows(const t_t *restrict column, const uint64_t *restrict bits,
    uint32_t count, seen_t *seen)
{
    uint32_t is;
    uint32_t r = 0;

    while (r < count) {
        is = (uint32_t)(bits[r >> 6] >> (r & 63)) & 1;
        seen->total += column[r].v & (0u - is);
        seen->on += is;
        r++;
    }
}

/*
** look_b
**
** Says the rows that are on, tested at every row: how many, and their sum.
*/
NATIVE_MEASURED BRIDGE_NATIVE(look_b)
{
    const bridge_query_t *q = bridge_native_get_query(api, ntoggle_reading);
    const uint32_t *span;
    seen_t seen = {};
    uint32_t i = 0;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 1);
        see_rows((const t_t *)(api->shared_base + span[2]),
            bridge_native_get_bits(api, span[2]), span[0], &seen);
        i++;
    }
    say(api, "by rows: on, and their sum", seen.on, seen.total);
    return (0);
}
