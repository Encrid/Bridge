/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "bits.h"
#include "schedule.h"

bool schedule_init_matrix(schedule_matrix_t *m, uint32_t n)
{
    m->n = n;
    m->words = bits_words(n);
    m->bits = calloc(schedule_get_matrix_words(n), sizeof(*m->bits));
    return (m->bits != nullptr);
}

bool schedule_reaches(const schedule_matrix_t *m, uint32_t i, uint32_t j)
{
    return (bits_has(&m->bits[(size_t)i * m->words], j));
}

void schedule_add_edge(schedule_matrix_t *m, uint32_t i, uint32_t j)
{
    uint64_t *row = &m->bits[(size_t)i * m->words];

    bits_or(row, &m->bits[(size_t)j * m->words], m->words);
    bits_set(row, j);
}

void schedule_join_through(schedule_matrix_t *m, uint32_t k)
{
    const uint64_t *via = &m->bits[(size_t)k * m->words];

    for (uint32_t i = 0; i < m->n; i++)
        if (schedule_reaches(m, i, k))
            bits_or(&m->bits[(size_t)i * m->words], via, m->words);
}
