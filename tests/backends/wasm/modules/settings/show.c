/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "settings.h"

static void say_title(uint32_t at)
{
    char text[sizeof(((settings_t *)0)->title)];
    uint32_t from = at + (uint32_t)offsetof(settings_t, title);
    uint32_t len = 0;
    uint32_t i = 0;

    while (i < sizeof(text)) {
        text[i] = (char)acc_ld_u8(from + i);
        i++;
    }
    while (len < sizeof(text) && text[len])
        len++;
    bridge_log(BRIDGE_LOG_INFO,
        &(bridge_line_t){ text, len, BRIDGE_GET(settings_t, small, at), 0 });
}

static void say_path(uint32_t at)
{
    char text[64];
    uint32_t len = BRIDGE_GET(settings_t, path.len, at);

    if (len > sizeof(text))
        len = sizeof(text);
    acc_from_shared(text, BRIDGE_GET(settings_t, path.at, at), len);
    bridge_log(BRIDGE_LOG_INFO, &(bridge_line_t){ text, len, 0, 0 });
}

static void say_wide(uint32_t at)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "flags ratio",
        (uint32_t)BRIDGE_GET(settings_t, flags, at),
        (uint32_t)BRIDGE_GET(settings_t, ratio, at));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "big",
        (uint32_t)(BRIDGE_GET(settings_t, big, at) >> 32), 0);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "path at", BRIDGE_GET(settings_t, path.at, at),
        0);
}

BRIDGE_EXPORT(show) int show(void)
{
    uint32_t at = bridge_get_single(settings);

    if (!at) {
        BRIDGE_LOG(BRIDGE_LOG_INFO, "no settings row", 0, 0);
        return (0);
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "vsync width",
        BRIDGE_GET(settings_t, vsync, at),
        (uint32_t)BRIDGE_GET(settings_t, width, at));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "scale mode",
        (uint32_t)(BRIDGE_GET(settings_t, scale, at) * 100.0f),
        (uint32_t)BRIDGE_GET(settings_t, mode, at));
    say_title(at);
    say_path(at);
    say_wide(at);
    return (0);
}
