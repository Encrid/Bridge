/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "greet.h"
#include "extra.h"
#include "outside.h"

int main(void)
{
    return ((greeting() == nullptr) + (outside() == nullptr) + EXTRA_STATUS);
}
