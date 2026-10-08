/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/region.h -- a stretch of the shared block handed out in pieces and
** taken back: the column region, and the persistent arena.
**
** What is free is kept outside the block, where no module's stray write
** reaches it: gaps, by offset from the region's start, in offset order and
** joined to their neighbours. A request takes the smallest gap that holds it
** before it takes fresh space. Nothing is ever taken from outside the region:
** a request it cannot meet is refused.
**
** budgets.md, *What running out does*
*/

#ifndef BRIDGE_SRC_REGION_H
    #define BRIDGE_SRC_REGION_H

    #include <stdint.h>

typedef struct {
    uint32_t at;
    uint32_t len;
} range_t;

/*
** A region. Everything past `used` has never been handed out, and `room` is
** how many gaps the list has memory for.
*/
typedef struct {
    range_t *gaps;
    uint32_t count;
    uint32_t room;
    uint32_t used;
    uint32_t len;
} region_t;

/*
** region_make_room
**
** Makes room in the list for the gaps that `ranges` ranges handed out can
** leave: one after each, and one before the first. false if there is not the
** memory.
*/
[[nodiscard]] bool region_make_room(region_t *r, uint32_t ranges);

/*
** region_drop_gap
**
** Takes gap `i` out of the list.
*/
void region_drop_gap(region_t *r, uint32_t i);

/*
** region_give
**
** Gives a piece back: it becomes a gap, joined to the gaps it touches, and a
** gap at the top is space never used again.
*/
void region_give(region_t *r, uint32_t at, uint32_t len);

/*
** region_take
**
** Takes a piece of `len` bytes: 1 and where it is, or 0 if the region has
** none that long.
*/
[[nodiscard]] int region_take(region_t *r, uint32_t len, uint32_t *at);

/*
** region_grow
**
** Makes the piece at `at` longer where it stands, from `had` bytes to `want`.
** false, and nothing changed, if what follows it is not free.
*/
[[nodiscard]] bool region_grow(region_t *r, uint32_t at, uint32_t had,
    uint32_t want);

/*
** region_get_most
**
** Answers the longest piece the region could give now.
*/
uint32_t region_get_most(const region_t *r);

#endif
