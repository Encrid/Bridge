/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "block.h"

static module_t *merge_work(module_t *a, module_t *b)
{
    module_t *head = nullptr;
    module_t **tail = &head;

    while (a && b) {
        if (a->place <= b->place) {
            *tail = a;
            a = a->next_work;
        } else {
            *tail = b;
            b = b->next_work;
        }
        tail = &(*tail)->next_work;
    }
    *tail = a ? a : b;
    return (head);
}

static module_t *cut_run(module_t **list, uint32_t length)
{
    module_t *head = *list;
    module_t *m = head;

    for (uint32_t i = 1; m && i < length; i++)
        m = m->next_work;
    *list = m ? m->next_work : nullptr;
    if (m)
        m->next_work = nullptr;
    return (head);
}

static module_t **find_end(module_t **tail)
{
    while (*tail)
        tail = &(*tail)->next_work;
    return (tail);
}

module_t *block_sort_list(module_t *list)
{
    module_t **tail;
    module_t *done;
    module_t *a;
    uint32_t merges = 2;

    for (uint32_t length = 1; merges > 1; length *= 2) {
        merges = 0;
        done = nullptr;
        tail = &done;
        while (list) {
            a = cut_run(&list, length);
            *tail = merge_work(a, cut_run(&list, length));
            tail = find_end(tail);
            merges++;
        }
        list = done;
    }
    return (list);
}

void block_sort_work(bridge_world_t *w)
{
    if (!w->work_mixed)
        return;
    w->work = block_sort_list(w->work);
    w->work_mixed = false;
}
