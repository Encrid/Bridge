/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/native_reach.h -- what a native test module reads to
** reach an entity by its id and not by a walk: the index of the entities,
** and where each table keeps each column.
*/

#ifndef TEST_NATIVE_REACH_H
    #define TEST_NATIVE_REACH_H

    #include "bridge_native.h"

/*
** get_entity_index
**
** Answers the index of the entities: the record of the entity whose id's
** low 32 bits are e is record e, and holds its generation, its table and
** its row.
*/
static inline bridge_entity_t *get_entity_index(const bridge_api_t *api)
{
    const bridge_header_t *h = bridge_native_get_header(api);

    return ((bridge_entity_t *)(api->shared_base + h->entity_index));
}

/*
** get_column_bases
**
** Answers where each table keeps each column, from the shared base: that
** of table t for the component of slot s is cell t * types + s, and is zero
** where the table does not carry the component.
*/
static inline const uint32_t *get_column_bases(const bridge_api_t *api)
{
    const bridge_header_t *h = bridge_native_get_header(api);

    return ((const uint32_t *)(api->private_base + h->column_base));
}

#endif
