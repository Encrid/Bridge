/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_FLOOR_H
    #define TEST_FLOOR_H

    #include <stdint.h>

typedef struct {
    uint32_t table;
    uint32_t row;
    uint32_t generation;
} floor_entity_t;

typedef struct {
    floor_entity_t *index;
    uint32_t *cells;
    uint64_t *given;
} floor_t;

#endif
