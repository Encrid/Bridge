/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "settings.h"

static void say_title(void)
{
    char text[sizeof(((settings_t *)0)->title)];
    uint32_t len = 0;
    uint32_t i = 0;

    while (i < sizeof(text)) {
        text[i] = (char)acc_ld_u8(settings, BRIDGE_AT(0, 0),
            (uint32_t)offsetof(settings_t, title) + i);
        i++;
    }
    while (len < sizeof(text) && text[len])
        len++;
    bridge_log(BRIDGE_LOG_INFO, &(bridge_line_t){ text, len,
        BRIDGE_GET(settings_t, small, settings, 0, 0), 0 });
}

static void say_path(void)
{
    char text[64];
    uint32_t len = BRIDGE_GET(settings_t, path.len, settings, 0, 0);

    if (len > sizeof(text))
        len = sizeof(text);
    acc_from_shared(text, BRIDGE_GET(settings_t, path.at, settings, 0, 0), len);
    bridge_log(BRIDGE_LOG_INFO, &(bridge_line_t){ text, len, 0, 0 });
}

static void say_wide(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "flags ratio",
        (uint32_t)BRIDGE_GET(settings_t, flags, settings, 0, 0),
        (uint32_t)BRIDGE_GET(settings_t, ratio, settings, 0, 0));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "big",
        (uint32_t)(BRIDGE_GET(settings_t, big, settings, 0, 0) >> 32), 0);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "path at",
        BRIDGE_GET(settings_t, path.at, settings, 0, 0), 0);
}

BRIDGE_EXPORT(show) int show(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "vsync width",
        BRIDGE_GET(settings_t, vsync, settings, 0, 0),
        (uint32_t)BRIDGE_GET(settings_t, width, settings, 0, 0));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "scale mode",
        (uint32_t)(BRIDGE_GET(settings_t, scale, settings, 0, 0) * 100.0f),
        (uint32_t)BRIDGE_GET(settings_t, mode, settings, 0, 0));
    say_title();
    say_path();
    say_wide();
    return (0);
}
