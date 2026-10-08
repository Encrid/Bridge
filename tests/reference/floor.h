/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/reference/floor.h -- what the floor is made of: where an entity
** is, and the arrays a value is reached through.
*/

#ifndef TEST_FLOOR_H
    #define TEST_FLOOR_H

    #include <stdint.h>

/*
** Where an entity is, and the generation its id must carry.
*/
typedef struct {
    uint32_t table;
    uint32_t row;
    uint32_t generation;
} floor_entity_t;

/*
** The arrays, each with a cell for every entity: where it is, its value,
** and a bit that says it has one.
*/
typedef struct {
    floor_entity_t *index;
    uint32_t *cells;
    uint64_t *given;
} floor_t;

#endif
