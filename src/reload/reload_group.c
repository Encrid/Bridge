/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "bridge_inside.h"
#include "reload.h"

static void undo_member(bridge_world_t *w, reloading_t *r)
{
    uint32_t i = 0;

    reload_undo(r);
    while (i < w->system_count) {
        if (w->systems[i]->m == r->old)
            abi_unpark(w, w->systems[i]);
        i++;
    }
}

static int validate_group(bridge_world_t *w, reloading_t *reloads,
    const bridge_artifact_t *artifacts, uint32_t count)
{
    uint32_t i = 0;
    uint32_t j;

    while (i < count) {
        reloads[i] = (reloading_t){ .w = w, .a = &artifacts[i] };
        if (reload_check(&reloads[i]))
            return (1);
        if (!(reloads[i].on->capabilities & BACKEND_SIDE_BY_SIDE))
            return (reload_refuse(&reloads[i], "a group needs every module on "
                "a backend that reloads side by side"));
        j = 0;
        while (j < i && strcmp(reloads[i].a->cfg.identity,
            reloads[j].a->cfg.identity))
            j++;
        if (j < i)
            return (reload_refuse(&reloads[i], "the group names it twice"));
        reloads[i].old->regrouping = true;
        i++;
    }
    return (0);
}

static uint32_t start_members(bridge_world_t *w, reloading_t *reloads,
    uint32_t count)
{
    module_start_t st;
    uint32_t i = 0;

    while (i < count) {
        st = (module_start_t){ reloads[i].a, reloads[i].demands, "reload" };
        if (reload_grow_share(&reloads[i]) || reload_make_record(&reloads[i]))
            return (i);
        reload_take_cursors(&reloads[i]);
        reload_retire(&reloads[i]);
        reloads[i].n->reloads = reloads[i].old;
        if (module_start(reloads[i].n, &st, &reloads[i].err)) {
            reload_refuse(&reloads[i], "%s", reloads[i].err.text);
            undo_member(w, &reloads[i]);
            return (i);
        }
        i++;
    }
    return (count);
}

static int group_fail(bridge_world_t *w, reloading_t *reloads, uint32_t done,
    uint32_t count)
{
    char why[256] = "";
    uint32_t i = 0;

    if (!count)
        say_error(w, "no module of the group is reloaded: %s",
            "the group is empty");
    while (i < done) {
        undo_member(w, &reloads[i]);
        i++;
    }
    for (i = 0; i < count; i++)
        if (reloads[i].old)
            reloads[i].old->regrouping = false;
    (void)schedule_build(w, why, sizeof(why));
    free(reloads);
    return (1);
}

int bridge_reload_group_now(bridge_world_t *w,
    const bridge_artifact_t *artifacts, uint32_t count)
{
    reloading_t *reloads = count ? calloc(count, sizeof(*reloads)) : nullptr;
    char why[256] = "";
    uint32_t done;

    if (count && !reloads)
        return (bridge_lacks(w, nullptr, "a group reload's records"));
    if (!count || validate_group(w, reloads, artifacts, count))
        return (group_fail(w, reloads, 0, count));
    done = start_members(w, reloads, count);
    if (done < count || schedule_build(w, why, sizeof(why))
        || reload_move_group(reloads, count, why, sizeof(why))) {
        if (why[0])
            say_error(w, "no module of the group is reloaded: %s", why);
        return (group_fail(w, reloads, done, count));
    }
    reload_commit_group(w, reloads, count);
    free(reloads);
    block_barrier(w);
    return (w->stopped_by ? 1 : 0);
}
