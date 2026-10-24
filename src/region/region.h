/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_REGION_H
    #define BRIDGE_SRC_REGION_H

    #include <stdint.h>

typedef struct {
    uint32_t at;
    uint32_t len;
} range_t;

typedef struct {
    range_t *gaps;
    uint32_t count;
    uint32_t room;
    uint32_t used;
    uint32_t peak;
    uint32_t len;
} region_t;

[[nodiscard]] bool region_make_room(region_t *r, uint32_t ranges);

void region_drop_gap(region_t *r, uint32_t i);

void region_give(region_t *r, uint32_t at, uint32_t len);

void region_bump(region_t *r, uint32_t by);

[[nodiscard]] int region_take(region_t *r, uint32_t len, uint32_t *at);

[[nodiscard]] bool region_grow(region_t *r, uint32_t at, uint32_t had,
    uint32_t want);

uint32_t region_get_most(const region_t *r);

#endif
