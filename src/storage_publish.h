/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage_publish.h -- what a module reads, written at a barrier: its
** column_base, its spans, its observers' lists.
**
** storage.md, *What a barrier writes for a module*
*/

#ifndef BRIDGE_SRC_STORAGE_PUBLISH_H
    #define BRIDGE_SRC_STORAGE_PUBLISH_H

    #include "storage_inside.h"

/*
** One module being written for.
**
**     cell      its column_base: [table][declared type], where the type's
**               column is in that table
**     kept      after it, a cell for each type it declared: where a
**               component that is not kept in a table is reached
**     at, end   where the next thing is written in its private memory, and
**               where its span region ends
**     all       a declaration changed, or it was never written for: every
**               table is written, and not only the ones touched
**     again     its spans are all written again, since they sit one after
**               another and one of its systems matches other tables than it
**               did
*/
typedef struct {
    storage_t *r;
    module_t *m;
    uint32_t *cell;
    uint32_t *kept;
    uint64_t at;
    uint64_t end;
    bool all;
    bool again;
} publish_t;

/*
** storage_publish_cells
**
** Writes a module's column_base, and where each component it declared that
** is not kept in a table is reached.
*/
void storage_publish_cells(publish_t *p);

/*
** storage_publish_spans
**
** Writes a module's spans: all of them, or the ones whose table was touched.
** 1 if they do not fit in its span region.
*/
[[nodiscard]] int storage_publish_spans(publish_t *p);

/*
** storage_publish_lists
**
** Writes each of a module's observers its list of who triggered it, when
** there is one or there was. 1 if they do not fit.
*/
[[nodiscard]] int storage_publish_lists(publish_t *p);

#endif
