/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_SAY_H
    #define TEST_SAY_H

    #include "test_module.h"

static inline void say(const test_api_t *api, const char *text, uint32_t x,
    uint32_t y)
{
    bridge_line_t line = { text, (uint32_t)strlen(text), x, y };

    test_log(api, BRIDGE_LOG_INFO, &line);
}

#endif
