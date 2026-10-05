/* The schedule: which systems conflict, what orders them, and who runs at once.
 * docs/bridge/run/schedule.md, *Order within a phase is never implicit*, and threads.md.
 *
 * ponytail: a dense reachability matrix per phase, closed in O(n^3) at every load. Fine for
 * the tens of systems a phase has; adjacency lists and one topological sort if a phase ever
 * holds hundreds. */
#include "inside.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Floyd-Warshall, on bytes */
static void close_over(uint8_t *reach, uint32_t n)
{
    uint32_t i, j, k;
    for (k = 0; k < n; k ++) {
        for (i = 0; i < n; i ++) {
            if (!reach[i * n + k]) continue;
            for (j = 0; j < n; j ++) {
                if (reach[k * n + j]) reach[i * n + j] = 1;
            }
        }
    }
}

/* The pairs a deployment states. A pair need not name anything loaded; what is refused here
 * is a pair that says nothing, and pairs that contradict each other. */
int schedule_order(bridge *b, const bridge_order *order, uint32_t count)
{
    const char **names = calloc(count ? count * 2 : 1, sizeof *names);
    uint8_t *reach = calloc(count ? (size_t)count * count * 4 : 1, 1);
    uint32_t n = 0, i, k, refused = 0;

    b->before = calloc(count ? count : 1, sizeof *b->before);
    b->after = calloc(count ? count : 1, sizeof *b->after);
    for (i = 0; i < count && !refused; i ++) {
        const char *pair[2] = { order[i].before, order[i].after };
        uint32_t at[2];
        if (!pair[0] || !pair[1] || !*pair[0] || !*pair[1] || !strcmp(pair[0], pair[1])) {
            say(b, NULL, NULL, BRIDGE_ERR, "[order] pair %u does not name two systems", i);
            refused = 1;
            break;
        }
        b->before[i] = copy(pair[0], strlen(pair[0]));
        b->after[i] = copy(pair[1], strlen(pair[1]));
        b->order_count = i + 1;
        for (k = 0; k < 2; k ++) {
            for (at[k] = 0; at[k] < n && strcmp(names[at[k]], pair[k]); at[k] ++) {}
            if (at[k] == n) names[n ++] = pair[k];
        }
        reach[at[0] * count * 2 + at[1]] = 1;
    }
    if (!refused && count) {
        close_over(reach, count * 2);
        for (i = 0; i < n && !refused; i ++) {
            if (!reach[i * count * 2 + i]) continue;
            say(b, NULL, NULL, BRIDGE_ERR, "[order] contradicts itself: '%s' would run before "
                "itself", names[i]);
            refused = 1;
        }
    }
    free(names);
    free(reach);
    return (int)refused;
}

/* the component both touch with one of them writing it, or NULL */
static const char *conflict(const bridge_system *a, const bridge_system *b)
{
    uint32_t i, j;
    for (i = 0; i < a->terms; i ++) {
        for (j = 0; j < b->terms; j ++) {
            if (a->touches[i] && a->touches[i] == b->touches[j] && (a->writes[i] || b->writes[j])) {
                return a->touches[i]->name;
            }
        }
    }
    return NULL;
}

/* Every phase, and the barrier after them as one more: edges, the refusal, then levels. */
int schedule_build(bridge *b, char *why, size_t why_len)
{
    bridge_system **in = calloc(b->system_count ? b->system_count : 1, sizeof *in);
    uint8_t *reach = calloc(b->system_count ? (size_t)b->system_count * b->system_count : 1, 1);
    uint32_t phase, i, j, k, refused = 0;

    for (phase = 0; phase <= b->phase_count && !refused; phase ++) {
        const char *where = phase < b->phase_count ? b->phases[phase] : NULL;
        uint32_t n = 0;
        for (i = 0; i < b->system_count; i ++) {
            if (b->systems[i]->phase == phase) in[n ++] = b->systems[i];
        }
        memset(reach, 0, (size_t)n * n);

        for (k = 0; k < b->order_count; k ++) {
            for (i = 0; i < n && strcmp(in[i]->full, b->before[k]); i ++) {}
            for (j = 0; j < n && strcmp(in[j]->full, b->after[k]); j ++) {}
            if (i < n && j < n) reach[i * n + j] = 1;
        }
        close_over(reach, n);
        /* a module orders its own systems by registering them, unless [order] says otherwise */
        for (i = 0; i < n; i ++) {
            for (j = i + 1; j < n; j ++) {
                if (in[i]->m == in[j]->m && conflict(in[i], in[j]) && !reach[j * n + i]) {
                    reach[i * n + j] = 1;
                }
            }
        }
        close_over(reach, n);

        for (i = 0; i < n && !refused; i ++) {
            if (reach[i * n + i]) {
                snprintf(why, why_len, "[order] has '%.100s' run before itself", in[i]->full);
                refused = 1;
            }
            for (j = i + 1; j < n && !refused; j ++) {
                const char *shared = conflict(in[i], in[j]);
                if (!shared || in[i]->m == in[j]->m || reach[i * n + j] || reach[j * n + i]) continue;
                snprintf(why, why_len, "'%.80s' and '%.80s' both touch '%.60s' %s%.40s%s, one of them "
                    "writing it, and nothing orders them", in[i]->full, in[j]->full, shared,
                    where ? "in phase '" : "at the barrier", where ? where : "", where ? "'" : "");
                refused = 1;
            }
        }

        /* a level is everything whose predecessors are all in earlier ones */
        for (i = 0; i < n; i ++) in[i]->level = 0;
        for (k = 0; k < n && !refused; k ++) {
            for (i = 0; i < n; i ++) {
                for (j = 0; j < n; j ++) {
                    if (i != j && reach[i * n + j] && in[j]->level <= in[i]->level) {
                        in[j]->level = in[i]->level + 1;
                    }
                }
            }
        }
    }
    free(in);
    free(reach);
    return (int)refused;
}

/* ---- running: a level at a time, one job per module ---- */

typedef struct {
    module         *m;
    bridge_system **list;
    uint32_t        count;
} job;

/* A module's systems of one level, in the order it registered them, on one thread. */
static void run_job(void *arg)
{
    job *j = arg;
    module *m = j->m;
    bridge_header *h = (bridge_header *)m->priv;
    uint32_t i;
    for (i = 0; i < j->count; i ++) {
        bridge_system *s = j->list[i];
        bridge_system **slots = s->observer ? m->observers : m->systems;
        backend_err err = {0};
        int32_t status = 0;
        uint32_t slot = 0;
        while (slots[slot] != s) slot ++;
        /* validation.md: the culprit is the system that was running. One store per dispatch. */
        h->when = s->when;
        h->system = s->observer ? slot | 0x8000u : slot;
        /* fuel only depletes, so it is set again before every dispatch - limits.md */
        if (m->fuel > 0) m->on->api.instance_set_fuel(m->instance, (uint64_t)m->fuel);
        m->refused = 0;
        if (m->on->api.call(s->fn, &status, &err) || m->refused) {
            snprintf(s->why, sizeof s->why, "%s", m->refused ? m->why : err.text);
            s->failed = 1;
        } else if (status) {
            snprintf(s->why, sizeof s->why, "it answered %d", status);
            s->failed = 1;
        }
        h->system = UINT32_MAX;
    }
}

void schedule_run(bridge *b, uint32_t phase)
{
    uint32_t n = b->system_count, level, levels = 0, i, k, ordinal = 0;
    job *jobs;
    void **handed;
    uint8_t *callers;
    bridge_system **lists;
    if (!n) return;
    for (i = 0; i < n; i ++) {
        if (b->systems[i]->phase == phase && b->systems[i]->level >= levels) {
            levels = b->systems[i]->level + 1;
        }
    }
    if (!levels) return;
    jobs = calloc(n, sizeof *jobs);
    handed = calloc(n, sizeof *handed);
    callers = calloc(n, 1);
    lists = calloc(n, sizeof *lists);

    for (level = 0; level < levels; level ++) {
        uint32_t count = 0, listed = 0;
        /* a module's systems are gathered together, so that its job's list is one run */
        for (i = 0; i < n; i ++) {
            bridge_system *first = b->systems[i];
            job *j;
            for (k = 0; k < count && jobs[k].m != first->m; k ++) {}
            if (k < count) continue;
            j = &jobs[count];
            j->m = first->m;
            j->list = &lists[listed];
            j->count = 0;
            for (k = i; k < n; k ++) {
                bridge_system *s = b->systems[k];
                if (s->m != j->m || s->phase != phase || s->level != level || s->m->stopped
                    || (s->observer && !s->pending_count)) continue;
                s->when = b->barriers << 16 | (ordinal ++ & 0xffffu);
                s->failed = 0;
                j->list[j->count ++] = s;
            }
            if (!j->count) continue;
            listed += j->count;
            handed[count] = j;
            callers[count] = (uint8_t)j->m->pinned;
            count ++;
        }
        pool_run(b->pool, run_job, handed, callers, count);
        /* failure.md: the invocation is abandoned, and logged - here, so that the sink is
         * only ever called on the thread that asked for the phase */
        for (k = 0; k < count; k ++) {
            for (i = 0; i < jobs[k].count; i ++) {
                bridge_system *s = jobs[k].list[i];
                if (s->failed) {
                    say(b, s->m, s, BRIDGE_ERR, "%s '%s' failed: %s",
                        s->observer ? "observer" : "system", s->name, s->why);
                }
            }
        }
    }
    free(jobs);
    free(handed);
    free(callers);
    free(lists);
}
