/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef SWEEP_H
    #define SWEEP_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #ifndef _WIN32

int putenv(char *text);

    #endif

static inline void sweep_plan(const char *calls)
{
    static char plan[256];
    static unsigned turn;

    turn++;
    snprintf(plan, sizeof(plan), "C23_FAIL= turn=%u %s told=0", turn, calls);
    #ifdef _WIN32
    _putenv(plan);
    #else
    putenv(plan);
    #endif
}

static inline void sweep_refuse(const char *call, unsigned at)
{
    char calls[128];

    snprintf(calls, sizeof(calls), "%s=%u", call, at);
    sweep_plan(calls);
}

static inline bool sweep_is_refused(void)
{
    const char *all = getenv("C23_FAIL");

    return (all && strstr(all, " told=1") != nullptr);
}

#endif
