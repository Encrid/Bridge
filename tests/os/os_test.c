/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "os.h"

int main(void)
{
    void *base = os_reserve(8192, 4096);
    os_event_t *e;

    if (!base)
        return (printf("FAIL: reserve\n"), 1);
    if (os_commit(base, 8192))
        return (printf("FAIL: commit\n"), 1);
    os_release(base, 8192);
    e = os_event_create();
    if (!e)
        return (printf("FAIL: event\n"), 1);
    os_event_wait(e, OS_SECOND / 1000);
    os_event_set(e);
    os_event_wait(e, OS_SECOND / 1000);
    os_event_free(e);
    printf("os ok\n");
    return (0);
}
