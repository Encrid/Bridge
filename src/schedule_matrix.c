/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/schedule_matrix.c -- who runs before whom, as a row of bits a system.
*/

#include <stdlib.h>

#include "schedule.h"

/*
** schedule_init_matrix
**
** Makes an empty matrix for `n` systems. false if there was not the memory.
*/
bool schedule_init_matrix(schedule_matrix_t *m, uint32_t n)
{
    m->n = n;
    m->words = (n + 63) / 64;
    m->bits = calloc((size_t)n * m->words + 1, sizeof(*m->bits));
    return (m->bits != nullptr);
}

/*
** schedule_reaches
**
** Answers whether `i` runs before `j`.
*/
bool schedule_reaches(const schedule_matrix_t *m, uint32_t i, uint32_t j)
{
    uint64_t word = m->bits[(size_t)i * m->words + j / 64];

    return (word >> (j % 64) & 1);
}

/*
** schedule_add_edge
**
** States that `i` runs before `j`.
*/
void schedule_add_edge(schedule_matrix_t *m, uint32_t i, uint32_t j)
{
    m->bits[(size_t)i * m->words + j / 64] |= (uint64_t)1 << (j % 64);
}

/*
** join_through
**
** Makes every system that reaches `k` reach all that `k` does.
*/
static void join_through(schedule_matrix_t *m, uint32_t k)
{
    const uint64_t *via = &m->bits[(size_t)k * m->words];
    uint64_t *row = m->bits;
    uint32_t i = 0;
    uint32_t w;

    while (i < m->n) {
        w = schedule_reaches(m, i, k) ? 0 : m->words;
        while (w < m->words) {
            row[w] |= via[w];
            w++;
        }
        row += m->words;
        i++;
    }
}

/*
** schedule_close_matrix
**
** Makes whatever reaches a system reach all that it does: Warshall's
** closure, a system at a time.
*/
void schedule_close_matrix(schedule_matrix_t *m)
{
    uint32_t k = 0;

    while (k < m->n) {
        join_through(m, k);
        k++;
    }
}
