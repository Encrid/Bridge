/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/schema/schema_constants.c -- the enums and bitmasks a descriptor is
** read as. The integer its author stated is its width, and its range.
*/

#include "schema_test.h"

static const want_constants_t SCHEMA_CONSTANTS[] = {
    { BRIDGE_ENUM, "int32_t { Off, On, Adaptive }", 4, { 0, 1, 2 }, 3 },
    { BRIDGE_ENUM, "int32_t { A = 5, B, C = -1, D, }", 4, { 5, 6, -1, 0 }, 4 },
    { BRIDGE_ENUM, "int32_t { X = 0x10 }", 4, { 16 }, 1 },
    { BRIDGE_BITMASK, "uint32_t { Loud = 1, Wide = 2 }", 4, { 1, 2 }, 2 },
    { BRIDGE_BITMASK, "uint32_t { Top = 0x80000000 }", 4, { 2147483648 }, 1 },
    { BRIDGE_ENUM, "int8_t { Low = -128, High = 127 }", 1, { -128, 127 }, 2 },
    { BRIDGE_ENUM, "uint8_t { A = 254, B }", 1, { 254, 255 }, 2 },
    { BRIDGE_ENUM, "uint16_t { A = 65535 }", 2, { 65535 }, 1 },
    { BRIDGE_BITMASK, "uint8_t { Top = 0x80 }", 1, { 128 }, 1 },
    { BRIDGE_BITMASK, "uint64_t { Top = 0x8000000000000000 }", 8,
        { INT64_MIN }, 1 }
};

/*
** schema_run_constants
**
** Checks every enum and bitmask there is a row for.
*/
int schema_run_constants(void)
{
    constexpr size_t count = sizeof(SCHEMA_CONSTANTS)
        / sizeof(*SCHEMA_CONSTANTS);
    int failed = 0;
    size_t i = 0;

    while (i < count) {
        failed += schema_check_constants(&SCHEMA_CONSTANTS[i]);
        i++;
    }
    return (failed);
}
