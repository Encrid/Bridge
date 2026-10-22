/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <fcntl.h>
#include <io.h>
#include <sys/locking.h>
#include <sys/stat.h>

#include "os.h"

int os_lock(const char *path)
{
    int file = _open(path, _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE);

    if (file < 0)
        return (-1);
    if (!_locking(file, _LK_NBLCK, 1))
        return (0);
    _close(file);
    return (1);
}
