/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage.h"
#include "storage_test.h"

static void say_usage(const storage_t *r, const char *when)
{
    storage_usage_t u = storage_usage(r);

    printf("%s: %u entities of a peak of %u, %u tables\n", when, u.entities,
        u.entities_peak, u.tables);
}

static void read_single(storage_t *r)
{
    storage_type_t *one = storage_declare(r, 4, 4, BRIDGE_SINGLETON);
    uint32_t read[2] = {};
    uint32_t value = 7;

    storage_visit_values(r, one, storage_test_count_value, &read[0]);
    storage_give_single(r, one, &value, 4);
    storage_visit_values(r, one, storage_test_count_value, &read[1]);
    printf("values of a singleton read: %u with no value, %u with a value\n",
        read[0], read[1]);
}

enum storage_test_end storage_test_call_on_ids(storage_test_t *t)
{
    storage_t *r = t->w->storage;
    uint64_t ids[4] = {};

    say_usage(r, "at the start");
    read_single(r);
    (void)storage_reserve(r, ids, 4);
    storage_hand_out(r, ids, 2);
    storage_hand_out(r, ids, 2);
    storage_release(r, ids + 2, 2);
    storage_hand_out(r, ids + 2, 1);
    say_usage(r, "after 4 ids, 2 of them released");
    printf("alive: no id %d, an id beyond the last %d, a handed id %d, a "
        "released id %d\n", storage_is_alive(r, 0),
        storage_is_alive(r, 1u << 30), storage_is_alive(r, ids[0]),
        storage_is_alive(r, ids[3]));
    storage_validate_baseline(r);
    return (STORAGE_TEST_DONE);
}
