/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "os.h"

char *os_path_whole(const char *path)
{
    return (_fullpath(nullptr, path, 0));
}
