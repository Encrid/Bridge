/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_OFFSETS_ACC_H
    #define TEST_OFFSETS_ACC_H

    #include <stddef.h>

    #include "test_module_api.h"

    #define BRIDGE_EXPORT(name) TEST_EXPORT

    #define BRIDGE_HEADER(field) acc_own_32(offsetof(bridge_header_t, field))

TEST_EXPORT void test_keep_table(const test_api_t *api);
const test_api_t *test_get_table(void);

uint32_t bridge_declare(uint32_t at);
uint32_t bridge_add_system(uint32_t at);
uint32_t bridge_observe(uint32_t at);
uint64_t bridge_spawn(void);
void bridge_destroy(uint64_t id);
void bridge_attach(uint64_t id, uint32_t slot, uint32_t ptr);
void bridge_detach(uint64_t id, uint32_t slot);
uint32_t bridge_alloc(uint32_t len);

void acc_trap(void);
uint32_t acc_resolve(uint32_t slot, uint32_t a, uint32_t b, uint32_t field,
    uint32_t width, uint32_t store);
uint32_t acc_toggle_bits(uint32_t slot, uint32_t table, uint32_t row);

int32_t acc_ld_i8(uint32_t slot, uint32_t a, uint32_t b, uint32_t field);
uint32_t acc_ld_u8(uint32_t slot, uint32_t a, uint32_t b, uint32_t field);
int32_t acc_ld_i16(uint32_t slot, uint32_t a, uint32_t b, uint32_t field);
uint32_t acc_ld_u16(uint32_t slot, uint32_t a, uint32_t b, uint32_t field);
uint32_t acc_ld_32(uint32_t slot, uint32_t a, uint32_t b, uint32_t field);
uint64_t acc_ld_64(uint32_t slot, uint32_t a, uint32_t b, uint32_t field);
float acc_ld_f32(uint32_t slot, uint32_t a, uint32_t b, uint32_t field);
double acc_ld_f64(uint32_t slot, uint32_t a, uint32_t b, uint32_t field);
void acc_st_8(uint32_t slot, uint32_t a, uint32_t b, uint32_t field, uint32_t v);
void acc_st_16(uint32_t slot, uint32_t a, uint32_t b, uint32_t field,
    uint32_t v);
void acc_st_32(uint32_t slot, uint32_t a, uint32_t b, uint32_t field,
    uint32_t v);
void acc_st_64(uint32_t slot, uint32_t a, uint32_t b, uint32_t field,
    uint64_t v);
void acc_st_f32(uint32_t slot, uint32_t a, uint32_t b, uint32_t field, float v);
void acc_st_f64(uint32_t slot, uint32_t a, uint32_t b, uint32_t field,
    double v);

uint64_t acc_entity(uint64_t id);
uint32_t acc_rows(uint32_t table);
uint64_t acc_who(uint32_t table, uint32_t row);
uint32_t acc_has(uint32_t slot, uint32_t id);
uint32_t acc_toggle(uint32_t slot, uint32_t table, uint32_t row);
void acc_toggle_set(uint32_t slot, uint32_t table, uint32_t row, uint32_t on);

uint32_t acc_own_32(uint32_t at);
uint64_t acc_own_64(uint32_t at);
void acc_own_st(uint32_t at, uint32_t v);
uint32_t acc_peek_32(uint32_t at);
uint64_t acc_peek_64(uint32_t at);

void acc_to_private(uint32_t to, const void *from, uint32_t len);
void acc_to_shared(uint32_t to, const void *from, uint32_t len);
void acc_from_shared(void *to, uint32_t from, uint32_t len);

#endif
