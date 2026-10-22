/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#include "os.h"

int os_lock(const char *path)
{
    int file = open(path, O_RDWR | O_CREAT, 0644);

    if (file < 0)
        return (-1);
    if (!flock(file, LOCK_EX | LOCK_NB))
        return (0);
    close(file);
    return (1);
}
