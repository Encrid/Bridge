/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRANCHY_H
    #define BRANCHY_H

static inline int get_sign(int value)
{
    if (value < 0)
        return (-1);
    return (value > 0 ? 1 : 0);
}

int count_unused(int value);

#endif
