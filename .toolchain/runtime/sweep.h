/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef SWEEP_H
    #define SWEEP_H

    #include <stdio.h>
    #include <stdlib.h>

    #ifndef _WIN32

int putenv(char *text);
int unsetenv(const char *name);

    #endif

static inline void sweep_refuse(const char *call, unsigned at)
{
    static char plan[128];
    static unsigned turn;

    turn++;
    snprintf(plan, sizeof(plan), "C23_FAIL= turn=%u %s=%u", turn, call, at);
    #ifdef _WIN32
    _putenv(plan);
    _putenv("C23_FAILED=");
    #else
    putenv(plan);
    unsetenv("C23_FAILED");
    #endif
}

static inline bool sweep_is_refused(void)
{
    return (getenv("C23_FAILED") != nullptr);
}

#endif
