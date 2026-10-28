/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_apply.h"
#include "storage_change.h"
#include "storage_forget.h"
#include "storage_room.h"
#include "storage_runs.h"
#include "storage_settle.h"

static void fold_all(storage_t *r)
{
    module_t *m;
    uint32_t at;
    uint32_t end;

    reset_marks(r->w);
    m = take_next_run(r->w, &at, &end);
    while (m) {
        fold_run(r, m, at, end);
        m = take_next_run(r->w, &at, &end);
    }
}

static void write_all(storage_t *r)
{
    module_t *m;
    change_t ch;
    uint32_t at;
    uint32_t end;

    reset_marks(r->w);
    m = take_next_run(r->w, &at, &end);
    while (m) {
        while (at < end) {
            memcpy(&ch, m->queue + at, sizeof(ch));
            write_value(r, m, &ch, m->queue + at + sizeof(ch));
            at += (uint32_t)sizeof(ch) + ((ch.size + 7u) & ~7u);
        }
        m = take_next_run(r->w, &at, &end);
    }
}

void storage_apply(storage_t *r)
{
    fold_all(r);
    reserve_all(r);
    tell_all(r);
    place_all(r);
    write_all(r);
    settle_all(r);
}
