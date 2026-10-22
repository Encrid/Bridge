/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_schema.h"
#include "say.h"

#ifdef MARKED
    #define MARK_FIELDS { uint32_t n; }

BRIDGE_SCHEMA_STRUCT_TYPE(mark_t, MARK_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(mark_t, MARK_FIELDS);

static uint32_t mark;
#endif

static void take_id_and_bytes(const test_api_t *api, const char *what)
{
    uint64_t id = api->spawn(api);
    uint32_t at = api->alloc(api, 8);

#ifdef MARKED
    api->attach(api, id, mark, BRIDGE_ZEROED);
#endif
    say(api, what, (uint32_t)id, at);
}

static void declare_mark([[maybe_unused]] const test_api_t *api)
{
#ifdef MARKED
    mark = TEST_COMPONENT(api, "dup:Mark", mark_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
#endif
}

TEST(init)
{
    declare_mark(api);
    return (0);
}

TEST(reload)
{
    declare_mark(api);
    take_id_and_bytes(api, "reload took");
    return (0);
}

TEST(shutdown)
{
    take_id_and_bytes(api, "shutdown took");
    return (0);
}
