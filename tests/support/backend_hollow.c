/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/support/backend_hollow.c -- a library that is of this backend seam
** and has no other entry of it: for the case that refuses such a backend
** by the entry it lacks.
*/

#include "bridge_backend.h"

/*
** backend_get_abi
**
** Answers this backend seam.
*/
BACKEND_EXPORT uint32_t backend_get_abi(void)
{
    return (BACKEND_ABI);
}
