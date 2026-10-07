/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/native_say.h -- a line of the log whose text is not
** a literal, for the native test modules that say what they saw.
*/

#ifndef TEST_NATIVE_SAY_H
    #define TEST_NATIVE_SAY_H

    #include "bridge_native.h"

/*
** say
**
** Writes `text` and two numbers into the module's log, at the level of
** information.
*/
static inline void say(const bridge_api_t *api, const char *text, uint32_t x,
    uint32_t y)
{
    bridge_line_t line = { text, (uint32_t)strlen(text), x, y };

    bridge_native_log(api, BRIDGE_LOG_INFO, &line);
}

#endif
