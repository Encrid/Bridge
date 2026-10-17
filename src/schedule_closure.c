/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

void schedule_close_matrix(schedule_matrix_t *m)
{
    uint32_t k = 0;

    while (k < m->n) {
        schedule_join_through(m, k);
        k++;
    }
}

uint32_t schedule_get_cycle(const schedule_matrix_t *m)
{
    uint32_t i = 0;

    while (i < m->n && !schedule_reaches(m, i, i))
        i++;
    return (i);
}
