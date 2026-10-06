/* The schedule: which systems conflict, what orders them, and who runs at once.
 * docs/bridge/run/schedule.md, *Order within a phase is never implicit*, and threads.md.
 *
 * Who must run before whom is a reachability matrix per phase, a row of bits per system,
 * closed by or-ing rows: n^3 / 64 word operations at a load, which for a phase of a thousand
 * systems is some sixteen million. Nothing here runs per frame. */
#include "inside.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { uint64_t *bits; uint32_t n, words; } matrix;

static void matrix_init(matrix *m, uint32_t n)
{
    m->n = n;
    m->words = (n + 63) / 64;
    m->bits = calloc((size_t)n * m->words + 1, sizeof *m->bits);
}

static int reaches(const matrix *m, uint32_t i, uint32_t j)
{
    return (int)(m->bits[(size_t)i * m->words + j / 64] >> (j % 64) & 1);
}

static void edge(matrix *m, uint32_t i, uint32_t j)
{
    m->bits[(size_t)i * m->words + j / 64] |= (uint64_t)1 << (j % 64);
}

/* Warshall: whatever reaches k reaches all that k does */
static void close_over(matrix *m)
{
    uint32_t i, k, w;
    for (k = 0; k < m->n; k ++) {
        const uint64_t *via = &m->bits[(size_t)k * m->words];
        for (i = 0; i < m->n; i ++) {
            uint64_t *row = &m->bits[(size_t)i * m->words];
            if (!reaches(m, i, k)) continue;
            for (w = 0; w < m->words; w ++) row[w] |= via[w];
        }
    }
}

/* The pairs a deployment states. A pair need not name anything loaded; what is refused here
 * is a pair that says nothing, and pairs that contradict each other. */
int schedule_order(bridge *b, const bridge_order *order, uint32_t count)
{
    const char **names = calloc(count ? count * 2 : 1, sizeof *names);
    matrix reach;
    uint32_t n = 0, i, k, refused = 0;

    matrix_init(&reach, count * 2);

    b->before = calloc(count ? count : 1, sizeof *b->before);
    b->after = calloc(count ? count : 1, sizeof *b->after);
    for (i = 0; i < count && !refused; i ++) {
        const char *pair[2] = { order[i].before, order[i].after };
        uint32_t at[2];
        if (!pair[0] || !pair[1] || !*pair[0] || !*pair[1] || !strcmp(pair[0], pair[1])) {
            say(b, nullptr, nullptr, BRIDGE_ERR, "[order] pair %u does not name two systems", i);
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
        edge(&reach, at[0], at[1]);
    }
    if (!refused && count) {
        close_over(&reach);
        for (i = 0; i < n && !refused; i ++) {
            if (!reaches(&reach, i, i)) continue;
            say(b, nullptr, nullptr, BRIDGE_ERR, "[order] contradicts itself: '%s' would run before "
                "itself", names[i]);
            refused = 1;
        }
    }
    free(names);
    free(reach.bits);
    return (int)refused;
}

/* the component both touch with one of them writing it, or that both attach or detach, or
 * nullptr. `changing` says which */
static const char *conflict(const bridge_system *a, const bridge_system *b, bool *changing)
{
    uint32_t i, j;
    *changing = true;
    for (i = 0; i < a->change_count; i ++) {
        for (j = 0; j < b->change_count; j ++) {
            if (a->changes[i] == b->changes[j]) return a->changes[i]->name;
        }
    }
    *changing = false;
    for (i = 0; i < a->terms; i ++) {
        for (j = 0; j < b->terms; j ++) {
            if (a->term[i].touches && a->term[i].touches == b->term[j].touches
                && (a->term[i].writes || b->term[j].writes)) {
                return a->term[i].touches->name;
            }
        }
    }
    return nullptr;
}

/* Every phase, and the barrier after them as one more: edges, the refusal, then levels. */
int schedule_build(bridge *b, char *why, size_t why_len)
{
    bridge_system **in = calloc(b->system_count ? b->system_count : 1, sizeof *in);
    uint32_t phase, i, j, k, refused = 0;
    bool changing;

    for (phase = 0; phase <= b->phase_count && !refused; phase ++) {
        const char *where = phase < b->phase_count ? b->phases[phase] : nullptr;
        uint32_t n = 0;
        matrix reach;
        for (i = 0; i < b->system_count; i ++) {
            if (b->systems[i]->phase == phase && !b->systems[i]->parked) in[n ++] = b->systems[i];
        }
        matrix_init(&reach, n);

        for (k = 0; k < b->order_count; k ++) {
            for (i = 0; i < n && strcmp(in[i]->full, b->before[k]); i ++) {}
            for (j = 0; j < n && strcmp(in[j]->full, b->after[k]); j ++) {}
            if (i < n && j < n) edge(&reach, i, j);
        }
        close_over(&reach);
        /* a module orders its own systems by registering them, unless [order] says otherwise */
        for (i = 0; i < n; i ++) {
            for (j = i + 1; j < n; j ++) {
                if (in[i]->m == in[j]->m && conflict(in[i], in[j], &changing)
                    && !reaches(&reach, j, i)) {
                    edge(&reach, i, j);
                }
            }
        }
        close_over(&reach);

        for (i = 0; i < n && !refused; i ++) {
            if (reaches(&reach, i, i)) {
                snprintf(why, why_len, "[order] has '%.100s' run before itself", in[i]->full);
                refused = 1;
            }
            for (j = i + 1; j < n && !refused; j ++) {
                const char *shared = conflict(in[i], in[j], &changing);
                if (!shared || in[i]->m == in[j]->m || reaches(&reach, i, j)
                    || reaches(&reach, j, i)) continue;
                snprintf(why, why_len, "'%.80s' and '%.80s' both %s '%.60s' %s%.40s%s%s, and nothing "
                    "orders them", in[i]->full, in[j]->full, changing ? "attach or detach" : "touch",
                    shared, where ? "in phase '" : "at the barrier", where ? where : "",
                    where ? "'" : "", changing ? "" : ", one of them writing it");
                refused = 1;
            }
        }

        /* a level is everything whose predecessors are all in earlier ones */
        for (i = 0; i < n; i ++) in[i]->level = 0;
        for (k = 0; k < n && !refused; k ++) {
            for (i = 0; i < n; i ++) {
                for (j = 0; j < n; j ++) {
                    if (i != j && reaches(&reach, i, j) && in[j]->level <= in[i]->level) {
                        in[j]->level = in[i]->level + 1;
                    }
                }
            }
        }
        free(reach.bits);
    }
    free(in);
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
    auto h = (bridge_header *)m->priv;
    uint32_t i;
    for (i = 0; i < j->count; i ++) {
        bridge_system *s = j->list[i];
        bridge_system **slots = s->observer ? m->observers : m->systems;
        backend_err err = {};
        int32_t status = 0;
        uint32_t slot = 0;
        while (slots[slot] != s) slot ++;
        /* validation.md: the culprit is the system that was running. One store per dispatch. */
        h->when = s->when;
        m->running = s;
        m->turn = s->turn;
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
        m->running = nullptr;
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
                    || s->parked || (s->observer && !s->pending_count)) continue;
                /* threads.md: an observer ran at the last barrier, before anything since */
                s->turn = (phase == b->phase_count ? 0 : TURN_SYSTEM) | (ordinal & 0xffffu);
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
                /* failure.md, *The budget*: counted a system at a time, so that one that
                 * always fails is not forgiven by another of its module's that never does */
                if (!s->failed) {
                    if (s->failures) s->failures --;
                    continue;
                }
                s->failures ++;
                if (s->m->max_failures > 0 && s->failures >= s->m->max_failures && !s->m->reached) {
                    s->m->reached = s;
                }
                say(b, s->m, s, BRIDGE_ERR, "%s '%s' failed: %s",
                    s->observer ? "observer" : "system", s->name, s->why);
            }
        }
    }
    free(jobs);
    free(handed);
    free(callers);
    free(lists);
}

/* failure.md, *The budget*: a module is stopped at the barrier after one of its systems
 * reached its number, or after it did itself, by entities it changed that could not be
 * placed. Counted in invocations and in barriers and never in time, so that the same history
 * stops the same module at the same place every run. */
void schedule_account(bridge *b)
{
    module *m;
    for (m = b->modules; m; m = m->next) {
        const bridge_system *s = m->reached;
        if (m->unplaced_now) m->failures ++;
        else if (m->changed_now && m->failures) m->failures --;
        m->changed_now = m->unplaced_now = 0;
        m->reached = nullptr;
        if (m->stopped || m->max_failures <= 0 || (!s && m->failures < m->max_failures)) continue;
        m->stopped = 1;
        if (s) {
            say(b, m, nullptr, BRIDGE_ERR, "module '%s' is stopped: its %s '%s' reached its "
                "max_system_failures of %lld", m->identity, s->observer ? "observer" : "system",
                s->name, (long long)m->max_failures);
        } else {
            say(b, m, nullptr, BRIDGE_ERR, "module '%s' is stopped: it reached its "
                "max_system_failures of %lld, by entities it changed that could not be placed",
                m->identity, (long long)m->max_failures);
        }
        /* fail covers a stop as it covers a load: the deployment does not run without it */
        if (m->on_error == BRIDGE_FAIL) bridge_stop(b, m->identity, "was stopped");
    }
}
