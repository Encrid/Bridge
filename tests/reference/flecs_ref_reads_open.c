/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/reference/flecs_ref_reads_open.c -- what the reads are made over,
** none of it timed.
*/

#include "flecs_ref.h"

/*
** flecs_ref_open_reads
**
** Makes ROWS entities that carry the small component in a table and the
** sparse one, which does not fragment, and the three queries: the column,
** the sparse component, and both.
*/
void flecs_ref_open_reads(flecs_ref_t *r, ecs_query_t **q)
{
    ECS_COMPONENT(r->w, small_t);
    ECS_COMPONENT(r->w, sparse_t);
    r->id = ecs_id(sparse_t);
    ecs_add_id(r->w, r->id, EcsSparse);
    ecs_add_id(r->w, r->id, EcsDontFragment);
    r->ids = flecs_ref_make_ids(r->w, ecs_id(small_t), r->count);
    flecs_ref_set_ones(r);
    q[0] = flecs_ref_make_query(r->w, ecs_id(small_t), 0);
    q[1] = flecs_ref_make_query(r->w, r->id, 0);
    q[2] = flecs_ref_make_query(r->w, ecs_id(small_t), r->id);
}
