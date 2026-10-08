/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nstress/nstress.h -- what the sources of nstress
** share: the one type its eight components have, what init was answered,
** and the module's own account of what its entities ought to carry.
*/

#ifndef TEST_NSTRESS_H
    #define TEST_NSTRESS_H

    #include "bridge_native.h"

    #define V_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(v_t, V_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(v_t, V_FIELDS);

constexpr uint32_t KINDS = 8;
constexpr uint32_t SLOTS = 50'000;
constexpr uint32_t CHANGES = 20'000;

/*
** The slot of each component, and the query of the system that sees it.
*/
extern uint32_t nstress_kinds[KINDS];
extern uint32_t nstress_seeing[KINDS];

/*
** The account: the id each of the module's entities has, or zero for one
** it has not made or has destroyed; which components it ought to carry, a
** bit each; the value it ought to have for each; and how many times what
** was found disagreed with that.
*/
extern uint64_t nstress_ids[SLOTS];
extern uint8_t nstress_has[SLOTS];
extern uint32_t nstress_values[SLOTS][KINDS];
extern uint32_t nstress_wrong;

/*
** nstress_check_entities
**
** Looks at every entity by its id: where the index says it is, and what
** column_base says is there. Counts what disagrees with the account, and
** every twenty-fifth time says how many entities are alive, a digest of the
** account, and the disagreements so far.
*/
void nstress_check_entities(const bridge_api_t *api);

/*
** nstress_check_kind
**
** Looks at component `kind` by its spans: as many rows as carry it, holding
** what they should. Counts a disagreement with the account, and for the
** first component checks every entity as well. Answers 0.
*/
int nstress_check_kind(const bridge_api_t *api, uint32_t kind);

#endif
