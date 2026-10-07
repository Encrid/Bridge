/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/support/native_other_abi.c -- the native backend's answer to
** backend_get_abi, as a backend built for another backend seam would give
** it: for the case that refuses such a backend.
*/

#include "bridge_backend.h"

/*
** backend_get_abi
**
** Answers a backend seam that is not this one.
*/
BACKEND_EXPORT uint32_t backend_get_abi(void)
{
    return (BACKEND_ABI + 1);
}
