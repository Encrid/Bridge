/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "schedule.h"

bool schedule_init_matrix(schedule_matrix_t *m, uint32_t n)
{
    m->n = n;
    m->words = (uint32_t)(((uint64_t)n + 63) / 64);
    m->bits = calloc((size_t)n * m->words + 1, sizeof(*m->bits));
    return (m->bits != nullptr);
}

bool schedule_reaches(const schedule_matrix_t *m, uint32_t i, uint32_t j)
{
    uint64_t word = m->bits[(size_t)i * m->words + j / 64];

    return (word >> (j % 64) & 1);
}

void schedule_add_edge(schedule_matrix_t *m, uint32_t i, uint32_t j)
{
    m->bits[(size_t)i * m->words + j / 64] |= (uint64_t)1 << (j % 64);
}

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

void schedule_close_matrix(schedule_matrix_t *m)
{
    uint32_t k = 0;

    while (k < m->n) {
        join_through(m, k);
        k++;
    }
}
