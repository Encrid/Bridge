/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "idle.h"
#include "platform.h"

static int nap_once(const test_api_t *api)
{
    bridge_line_t line = { "nap", 3, 0, get_now_ms() };

    sleep_tenth();
    line.a0 = get_thread_id();
    test_log(api, BRIDGE_LOG_INFO, &line);
    line.text = "end";
    line.a0 = get_thread_id();
    line.a1 = get_now_ms();
    test_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

TEST(nap)
{
    return (nap_once(api));
}

TEST(doze)
{
    return (nap_once(api));
}

#ifdef OBSERVED

TEST(init)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    terms[0].slot = TEST_DECLARE_TAG(api, "test:Nap", BRIDGE_STRIP,
        BRIDGE_TABLE);
    TEST_OBSERVER(api, "nap", BRIDGE_ON_ENTER, terms, 1, nullptr, 0,
        nullptr, 0);
    TEST_OBSERVER(api, "doze", BRIDGE_ON_ENTER, terms, 1, nullptr, 0,
        nullptr, 0);
    api->attach(api, api->spawn(api), terms[0].slot, BRIDGE_ZEROED);
    return (0);
}

#else

TEST(init)
{
    int none = 0;

    TEST_ADD_SYSTEM(api, "nap", "update", &none, 0, nullptr, 0, nullptr, 0);
    #ifdef TWICE
    TEST_ADD_SYSTEM(api, "doze", "update", &none, 0, nullptr, 0, nullptr, 0);
    #endif
    return (0);
}

#endif
