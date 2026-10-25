/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <sys/stat.h>

int main(int count, char **words)
{
    struct stat about;

    if (count < 2 || stat(words[1], &about) != 0)
        return (1);
    return (0);
}
