/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/backend/tests_abi.c -- the backend seam the tests' backend answers
** to. A file of its own, so that a case builds this backend with another
** answer in its place.
*/

#include "tests_inside.h"

/*
** backend_get_abi
**
** Answers the backend seam this backend was built for.
*/
BACKEND_EXPORT uint32_t backend_get_abi(void)
{
    return (BACKEND_ABI);
}
