/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_PUBLISH_H
    #define BRIDGE_SRC_STORAGE_PUBLISH_H

    #include "storage_inside.h"

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

void storage_publish_cells(publish_t *p);

[[nodiscard]] int storage_publish_spans(publish_t *p);

[[nodiscard]] int storage_publish_lists(publish_t *p);

#endif
