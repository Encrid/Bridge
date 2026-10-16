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

uint32_t acc_row(uint32_t base, uint32_t row, uint32_t count, uint32_t stride);
void acc_trap(void);

int32_t acc_ld_i8(uint32_t at);
uint32_t acc_ld_u8(uint32_t at);
int32_t acc_ld_i16(uint32_t at);
uint32_t acc_ld_u16(uint32_t at);
uint32_t acc_ld_32(uint32_t at);
uint64_t acc_ld_64(uint32_t at);
float acc_ld_f32(uint32_t at);
double acc_ld_f64(uint32_t at);
void acc_st_8(uint32_t at, uint32_t v);
void acc_st_16(uint32_t at, uint32_t v);
void acc_st_32(uint32_t at, uint32_t v);
void acc_st_64(uint32_t at, uint64_t v);
void acc_st_f32(uint32_t at, float v);
void acc_st_f64(uint32_t at, double v);

uint32_t acc_own_32(uint32_t at);
uint64_t acc_own_64(uint32_t at);
void acc_own_st(uint32_t at, uint32_t v);

void acc_to_private(uint32_t to, const void *from, uint32_t len);
void acc_to_shared(uint32_t to, const void *from, uint32_t len);
void acc_from_shared(void *to, uint32_t from, uint32_t len);

#endif
