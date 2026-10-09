/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/measured.h -- for the function of a
** test module whose loop a test times.
*/

#ifndef TEST_MEASURED_H
    #define TEST_MEASURED_H

/*
** A function whose loop is timed begins on a cache line of 64 bytes. Its
** loop is then at the same place in a line whatever the linker puts ahead
** of the function, so what the loop costs does not change when another
** function of the module does. Without it a loop of a few instructions was
** measured at up to twice its cost after a function ahead of it changed
** length by three bytes.
*/
    #define TEST_MEASURED [[gnu::aligned(64)]]

#endif
