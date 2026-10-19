/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "bridge_native.h"

[[gnu::section(MANIFEST_SECTION), gnu::used, gnu::retain]]
static const char WHO[] = MANIFEST;

static bool said;

#ifdef WAIT

static void go_on(void)
{
    char answer[64];

    printf("? go on\n");
    fflush(stdout);
    if (!fgets(answer, sizeof(answer), stdin))
        answer[0] = 0;
}

#endif

BRIDGE_NATIVE(say)
{
    bridge_line_t line = { "knock", 5, 0, 0 };

#ifdef WAIT
    if (!said)
        go_on();
#endif
    if (!said)
        bridge_native_log(api, BRIDGE_LOG_INFO, &line);
    said = true;
    return (0);
}

BRIDGE_NATIVE(init)
{
    int none = 0;

    BRIDGE_NATIVE_ADD_SYSTEM(api, "say", "update", &none, 0, nullptr, 0);
    return (0);
}

BRIDGE_NATIVE(shutdown)
{
    return (0);
}

BRIDGE_NATIVE(reload)
{
    return (0);
}
