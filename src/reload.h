/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_RELOAD_H
    #define BRIDGE_SRC_RELOAD_H

    #include "module.h"

typedef struct {
    bridge_t *b;
    const bridge_artifact_t *a;
    open_backend_t *on;
    enum backend_capability demands;
    module_t *old;
    module_t *n;
    uint32_t extra;
    uint32_t extra_at;
    range_t last;
    bool in_place;
    bool side_by_side;
    backend_err_t err;
} reloading_t;

[[gnu::format(printf, 2, 3)]] int reload_refuse(const reloading_t *r,
    const char *format, ...);

[[nodiscard]] int reload_check(reloading_t *r);

[[nodiscard]] int reload_grow_share(reloading_t *r);

void reload_give_back(reloading_t *r);

[[nodiscard]] int reload_make_record(reloading_t *r);

[[nodiscard]] int reload_move_rows(reloading_t *r);

int reload_fail(reloading_t *r);

#endif
