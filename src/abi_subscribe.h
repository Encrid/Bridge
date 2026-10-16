/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_ABI_SUBSCRIBE_H
    #define BRIDGE_SRC_ABI_SUBSCRIBE_H

    #include "abi_declare.h"

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

int abi_check_subscriber(subscribing_t *g);

int abi_alloc_subscriber(subscribing_t *g);

void abi_free_subscriber(subscribing_t *g);

int abi_check_term(subscribing_t *g, uint32_t i, const bridge_term_t *t);

int abi_subscribe(subscribing_t *g, uint64_t *r);

#endif
