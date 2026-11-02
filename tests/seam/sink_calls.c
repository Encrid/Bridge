/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "cases.h"

static bool called;

static int call_world(host_t *h, const char *name)
{
    bridge_module_info_t list[4];
    bridge_artifact_t a;
    int count;

    if (!strcmp(name, "bridge_load")) {
        test_state_module(&a, "test:other@0.1.0", "chatty");
        return (bridge_load(h->w, &a));
    }
    if (!strcmp(name, "bridge_reload")) {
        test_state_module(&a, "test:chatty@0.1.0", "chatty");
        return (bridge_reload(h->w, &a, 1));
    }
    if (!strcmp(name, "bridge_unload"))
        return (bridge_unload(h->w, "test:chatty@0.1.0"));
    if (!strcmp(name, "bridge_run_phase"))
        return (bridge_run_phase(h->w, "update"));
    if (!strcmp(name, "bridge_destroy"))
        return (bridge_destroy(h->w));
    if (!strcmp(name, "bridge_profile"))
        return (bridge_profile(h->w, nullptr, 0));
    if (!strcmp(name, "bridge_usage"))
        return (bridge_usage(h->w, nullptr, nullptr));
    if (!strcmp(name, "bridge_module_usage"))
        return (bridge_module_usage(h->w, nullptr, 0));
    count = bridge_list_modules(h->w, list, 4);
    if (count > 0)
        printf("listed %s\n", list[0].identity);
    return (count);
}

static void hear_and_call(void *ud, const bridge_record_t *r)
{
    host_t *h = ud;

    host_print_record(nullptr, r);
    if (called)
        return;
    called = true;
    printf("from the sink: %s gave %d\n", h->arg, call_world(h, h->arg));
}

typedef struct {
    host_t *h;
    const char *name;
    int result;
} elsewhere_t;

static void call_there(void *arg)
{
    elsewhere_t *e = arg;

    e->result = call_world(e->h, e->name);
}

static int call_from_thread(host_t *h, const char *name)
{
    elsewhere_t e = { h, name, 0 };
    host_thread_t t = { call_there, &e, nullptr };

    host_thread_start(&t);
    host_thread_join(&t);
    return (e.result);
}

static void hear_and_call_elsewhere(void *ud, const bridge_record_t *r)
{
    host_t *h = ud;

    host_print_record(nullptr, r);
    if (called)
        return;
    called = true;
    printf("from another thread during the call: %s gave %d\n", h->arg,
        call_from_thread(h, h->arg));
}

void test_set_calling_elsewhere(host_t *h)
{
    h->cfg.backend_count = 1;
    h->sink = hear_and_call_elsewhere;
    h->ud = h;
}

void test_set_calling(host_t *h)
{
    h->cfg.backend_count = 1;
    h->sink = hear_and_call;
    h->ud = h;
}

void test_call_from_sink(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "test:chatty@0.1.0", "chatty");
    called = true;
    host_load(h, &a);
    called = false;
    host_run_frames(h, 1);
    printf("the outer call ended\n");
    host_close(h);
}

void test_call_after_sink(host_t *h)
{
    const char *name = strchr(h->arg, ' ') + 1;
    bridge_artifact_t a;
    int result;

    if (host_open(h))
        return;
    test_state_module(&a, "test:chatty@0.1.0", "chatty");
    host_load(h, &a);
    host_run_frames(h, 1);
    printf("the outer call ended\n");
    if (!strncmp(h->arg, "elsewhere ", 10)) {
        result = call_from_thread(h, name);
        printf("from another thread: %s gave %d\n", name, result);
    } else {
        result = call_world(h, name);
        printf("from the host: %s gave %d\n", name, result);
    }
    if (!strcmp(name, "bridge_destroy") && !result)
        h->w = nullptr;
    host_close(h);
}
