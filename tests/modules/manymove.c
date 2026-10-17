/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"

    #define V_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(v_t, V_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(v_t, V_FIELDS);

constexpr uint32_t TYPES = 65;
constexpr uint32_t COUNT = 4;

static uint32_t slots[TYPES];

TEST(init)
{
    int none = 0;

    slots[0] = TEST_COMPONENT(api, "st:M0", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[1] = TEST_COMPONENT(api, "st:M1", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[2] = TEST_COMPONENT(api, "st:M2", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[3] = TEST_COMPONENT(api, "st:M3", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[4] = TEST_COMPONENT(api, "st:M4", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[5] = TEST_COMPONENT(api, "st:M5", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[6] = TEST_COMPONENT(api, "st:M6", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[7] = TEST_COMPONENT(api, "st:M7", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[8] = TEST_COMPONENT(api, "st:M8", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[9] = TEST_COMPONENT(api, "st:M9", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[10] = TEST_COMPONENT(api, "st:M10", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[11] = TEST_COMPONENT(api, "st:M11", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[12] = TEST_COMPONENT(api, "st:M12", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[13] = TEST_COMPONENT(api, "st:M13", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[14] = TEST_COMPONENT(api, "st:M14", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[15] = TEST_COMPONENT(api, "st:M15", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[16] = TEST_COMPONENT(api, "st:M16", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[17] = TEST_COMPONENT(api, "st:M17", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[18] = TEST_COMPONENT(api, "st:M18", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[19] = TEST_COMPONENT(api, "st:M19", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[20] = TEST_COMPONENT(api, "st:M20", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[21] = TEST_COMPONENT(api, "st:M21", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[22] = TEST_COMPONENT(api, "st:M22", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[23] = TEST_COMPONENT(api, "st:M23", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[24] = TEST_COMPONENT(api, "st:M24", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[25] = TEST_COMPONENT(api, "st:M25", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[26] = TEST_COMPONENT(api, "st:M26", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[27] = TEST_COMPONENT(api, "st:M27", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[28] = TEST_COMPONENT(api, "st:M28", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[29] = TEST_COMPONENT(api, "st:M29", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[30] = TEST_COMPONENT(api, "st:M30", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[31] = TEST_COMPONENT(api, "st:M31", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[32] = TEST_COMPONENT(api, "st:M32", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[33] = TEST_COMPONENT(api, "st:M33", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[34] = TEST_COMPONENT(api, "st:M34", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[35] = TEST_COMPONENT(api, "st:M35", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[36] = TEST_COMPONENT(api, "st:M36", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[37] = TEST_COMPONENT(api, "st:M37", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[38] = TEST_COMPONENT(api, "st:M38", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[39] = TEST_COMPONENT(api, "st:M39", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[40] = TEST_COMPONENT(api, "st:M40", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[41] = TEST_COMPONENT(api, "st:M41", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[42] = TEST_COMPONENT(api, "st:M42", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[43] = TEST_COMPONENT(api, "st:M43", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[44] = TEST_COMPONENT(api, "st:M44", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[45] = TEST_COMPONENT(api, "st:M45", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[46] = TEST_COMPONENT(api, "st:M46", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[47] = TEST_COMPONENT(api, "st:M47", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[48] = TEST_COMPONENT(api, "st:M48", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[49] = TEST_COMPONENT(api, "st:M49", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[50] = TEST_COMPONENT(api, "st:M50", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[51] = TEST_COMPONENT(api, "st:M51", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[52] = TEST_COMPONENT(api, "st:M52", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[53] = TEST_COMPONENT(api, "st:M53", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[54] = TEST_COMPONENT(api, "st:M54", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[55] = TEST_COMPONENT(api, "st:M55", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[56] = TEST_COMPONENT(api, "st:M56", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[57] = TEST_COMPONENT(api, "st:M57", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[58] = TEST_COMPONENT(api, "st:M58", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[59] = TEST_COMPONENT(api, "st:M59", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[60] = TEST_COMPONENT(api, "st:M60", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[61] = TEST_COMPONENT(api, "st:M61", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[62] = TEST_COMPONENT(api, "st:M62", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[63] = TEST_COMPONENT(api, "st:M63", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    slots[64] = TEST_COMPONENT(api, "st:M64", v_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    TEST_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, slots, TYPES, nullptr, 0);
    return (0);
}

TEST(reload)
{
    return (TEST_OWN(init)(api));
}

TEST(shutdown)
{
    return (0);
}

TEST(spawn)
{
    v_t value = { 7 };
    uint64_t id;
    uint32_t e = 0;
    uint32_t i;

    while (e < COUNT) {
        id = api->spawn(api);
        i = 0;
        while (i < TYPES) {
            TEST_ATTACH(api, id, slots[i], value);
            i++;
        }
        e++;
    }
    return (0);
}
