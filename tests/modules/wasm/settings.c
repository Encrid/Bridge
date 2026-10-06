/* A module whose settings are a component: a singleton, given its value by Bridge-ASM700 after
 * init, which a system reads back. docs/seam/host_api/config.md, *A module's own settings are a component*. */
#include "bridge_wasm.h"

BRIDGE_SCHEMA_ENUM(Mode, int8_t, { Off, On, Adaptive });
BRIDGE_SCHEMA_BITMASK(Flags, uint32_t, { Loud = 1, Wide = 2 });
BRIDGE_SCHEMA_STRUCT(Settings, {
    bool vsync;
    int32_t width;
    float scale;
    Mode mode;
    char title[16];
    bridge_text_t path;
    uint8_t small;
    Flags flags;
    double ratio;
    int64_t big;
});

/* a singleton, as the settings a module names must be; and a build that states otherwise */
#ifndef KEPT
#define KEPT BRIDGE_SINGLETON
#endif

static uint32_t settings;

BRIDGE_EXPORT(show) int show()
{
    char text[64];
    uint32_t at = bridge_get_single(settings), len, i;
    if (!at) {
        BRIDGE_LOG(BRIDGE_LOG_INFO, "no settings row", 0, 0);
        return 0;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "vsync width", BRIDGE_GET(Settings, vsync, at),
               (uint32_t)BRIDGE_GET(Settings, width, at));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "scale mode", (uint32_t)(BRIDGE_GET(Settings, scale, at) * 100.0f),
               (uint32_t)BRIDGE_GET(Settings, mode, at));
    for (i = 0; i < 16; i ++) text[i] = (char)acc_ld_u8(at + (uint32_t)offsetof(Settings, title) + i);
    for (len = 0; len < 16 && text[len]; len ++) {}
    bridge_log(BRIDGE_LOG_INFO, &(bridge_line_t){ text, len, BRIDGE_GET(Settings, small, at), 0 });
    len = BRIDGE_GET(Settings, path.len, at);
    if (len > sizeof text) len = sizeof text;
    acc_from_shared(text, BRIDGE_GET(Settings, path.at, at), len);
    bridge_log(BRIDGE_LOG_INFO, &(bridge_line_t){ text, len, 0, 0 });
    BRIDGE_LOG(BRIDGE_LOG_INFO, "flags ratio", (uint32_t)BRIDGE_GET(Settings, flags, at),
               (uint32_t)BRIDGE_GET(Settings, ratio, at));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "big", (uint32_t)(BRIDGE_GET(Settings, big, at) >> 32), 0);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "path at", BRIDGE_GET(Settings, path.at, at), 0);
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};
    BRIDGE_DECLARE(BRIDGE_ENUM, "game:Mode", Mode, BRIDGE_STRIP, BRIDGE_TABLE);
    BRIDGE_DECLARE(BRIDGE_BITMASK, "game:Flags", Flags, BRIDGE_STRIP, BRIDGE_TABLE);
    settings = BRIDGE_COMPONENT("game:Settings", Settings, BRIDGE_DELETE, KEPT);
    terms[0].slot = settings;
    BRIDGE_ADD_SYSTEM("show", "update", terms, 1, nullptr, 0);
    return 0;
}

/* over the row that is there: the same registration, or with DROP none of it */
BRIDGE_EXPORT(reload) int reload()
{
#ifdef DROP
    return 0;
#else
    return init();
#endif
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
