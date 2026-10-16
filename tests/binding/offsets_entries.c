/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "test_offsets_acc.h"

uint32_t bridge_declare(uint32_t at)
{
    const test_api_t *api = test_get_table();

    return (api->declare(api, at));
}

uint32_t bridge_add_system(uint32_t at)
{
    const test_api_t *api = test_get_table();

    return (api->add_system(api, at));
}

uint32_t bridge_observe(uint32_t at)
{
    const test_api_t *api = test_get_table();

    return (api->observe(api, at));
}

uint64_t bridge_spawn(void)
{
    const test_api_t *api = test_get_table();

    return (api->spawn(api));
}

void bridge_destroy(uint64_t id)
{
    const test_api_t *api = test_get_table();

    api->destroy(api, id);
}
