/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_ABI_H
    #define BRIDGE_SRC_ABI_H

    #include "inside.h"

typedef struct {
    const char *name;
    backend_fn_t fn;
    enum backend_type params[3];
    uint32_t param_count;
    enum backend_type results[1];
    uint32_t result_count;
} abi_entry_t;

[[gnu::format(printf, 2, 3)]] int abi_refuse(module_t *m, const char *format,
    ...);

[[nodiscard]] int abi_define(module_t *m, backend_err_t *err);

[[nodiscard]] bool abi_marks(module_t *m, uint32_t more);

void abi_park(bridge_t *b, bridge_system_t *s);

void abi_unpark(bridge_t *b, bridge_system_t *s);

void abi_hear(void *ctx, uint64_t id, enum bridge_event event);

int abi_declare(void *ud, const uint64_t *at, uint64_t *r);
int abi_add_system(void *ud, const uint64_t *at, uint64_t *r);
int abi_observe(void *ud, const uint64_t *at, uint64_t *r);

int abi_spawn(void *ud, const uint64_t *a, uint64_t *r);
int abi_destroy(void *ud, const uint64_t *a, uint64_t *r);
int abi_attach(void *ud, const uint64_t *a, uint64_t *r);
int abi_detach(void *ud, const uint64_t *a, uint64_t *r);
int abi_alloc(void *ud, const uint64_t *a, uint64_t *r);

#endif
