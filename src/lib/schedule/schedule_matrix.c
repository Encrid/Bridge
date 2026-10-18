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
    m->words = schedule_get_words(n);
    m->bits = calloc(schedule_get_matrix_words(n), sizeof(*m->bits));
    return (m->bits != nullptr);
}

bool schedule_reaches(const schedule_matrix_t *m, uint32_t i, uint32_t j)
{
    return (schedule_has_bit(&m->bits[(size_t)i * m->words], j));
}

void schedule_add_edge(schedule_matrix_t *m, uint32_t i, uint32_t j)
{
    uint64_t *row = &m->bits[(size_t)i * m->words];

    schedule_join_bits(row, &m->bits[(size_t)j * m->words], m->words);
    schedule_put_bit(row, j);
}

void schedule_join_through(schedule_matrix_t *m, uint32_t k)
{
    const uint64_t *via = &m->bits[(size_t)k * m->words];

    for (uint32_t i = 0; i < m->n; i++)
        if (schedule_reaches(m, i, k))
            schedule_join_bits(&m->bits[(size_t)i * m->words], via, m->words);
}
