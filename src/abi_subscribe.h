/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/abi_subscribe.h -- a system or an observer being registered: a query
** plus a function either way, so one registration serves both.
**
** query.md, *What a query may not ask for*
*/

#ifndef BRIDGE_SRC_ABI_SUBSCRIBE_H
    #define BRIDGE_SRC_ABI_SUBSCRIBE_H

    #include "abi_declare.h"

/*
** A system or an observer being registered.
**
**     name_at, name_len       where the module put its name
**     terms_at, count         its terms, each a bridge_term_t
**     changes_at, change_count
**                             the slots of what it attaches or detaches,
**                             each a u32
**     events                  zero for a system, and then `phase` is the
**                             phase it named
**     what                    "system" or "observer", as a refusal says it
**     terms, changes          the same, found in the scratch arena. A
**                             module may have put them at any offset
**     of                      the term being read, as a refusal names it
**     s, asked                what is made of it, and what storage is asked
*/
typedef struct {
    module_t *m;
    uint32_t name_at;
    uint32_t name_len;
    uint32_t terms_at;
    uint32_t count;
    uint32_t changes_at;
    uint32_t change_count;
    uint32_t phase;
    uint32_t events;
    const char *what;
    char name[128];
    const bridge_term_t *terms;
    const void *changes;
    char of[192];
    bridge_system_t *s;
    storage_term_t *asked;
} subscribing_t;

/*
** abi_check_subscriber
**
** Refuses a system or an observer that is one more than a budget allows,
** that states what lies outside the scratch arena, or whose name is
** reserved or taken, and copies its name. 1 with a refusal.
*/
int abi_check_subscriber(subscribing_t *g);

/*
** abi_alloc_subscriber
**
** Allocates everything a system or an observer needs, before any of it is
** kept. 1 with a refusal, and nothing left allocated.
**
** failure.md, *Out of memory*
*/
int abi_alloc_subscriber(subscribing_t *g);

/*
** abi_free_subscriber
**
** Frees what abi_alloc_subscriber allocated, of one that is refused.
*/
void abi_free_subscriber(subscribing_t *g);

/*
** abi_check_term
**
** Refuses term `i` if it asks for what a query may not: a slot never
** declared, an access or an operator unset or unknown, a write no grant
** allows, or a component that is kept where a system does not walk. 1 with
** a refusal.
*/
int abi_check_term(subscribing_t *g, uint32_t i, const bridge_term_t *t);

/*
** abi_subscribe
**
** Registers a system or an observer, and answers its slot in `r`. 1 with a
** refusal.
*/
int abi_subscribe(subscribing_t *g, uint64_t *r);

#endif
