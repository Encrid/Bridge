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

typedef struct { uint64_t *bits; uint32_t n, words; } matrix_t;

/* false if there was not the memory for it */
static bool matrix_init(matrix_t *m, uint32_t n)
{
    m->n = n;
    m->words = (n + 63) / 64;
    m->bits = calloc((size_t)n * m->words + 1, sizeof *m->bits);
    return m->bits != nullptr;
}

static int reaches(const matrix_t *m, uint32_t i, uint32_t j)
{
    return (int)(m->bits[(size_t)i * m->words + j / 64] >> (j % 64) & 1);
}

static void edge(matrix_t *m, uint32_t i, uint32_t j)
{
    m->bits[(size_t)i * m->words + j / 64] |= (uint64_t)1 << (j % 64);
}

/* Warshall: whatever reaches k reaches all that k does */
static void close_over(matrix_t *m)
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
int schedule_order(bridge_t *b, const bridge_order_t *order, uint32_t count)
{
    const char **names = calloc(count ? count * 2 : 1, sizeof *names);
    matrix_t reach;
    uint32_t n = 0, i, k, refused = 0;

    b->before = calloc(count ? count : 1, sizeof *b->before);
    b->after = calloc(count ? count : 1, sizeof *b->after);
    if (!matrix_init(&reach, count * 2) || !names || !b->before || !b->after) {
        refused = (uint32_t)bridge_lacks(b, nullptr, "the order a deployment states");
    }
    for (i = 0; i < count && !refused; i ++) {
        const char *pair[2] = { order[i].before, order[i].after };
        uint32_t at[2];
        if (!pair[0] || !pair[1] || !*pair[0] || !*pair[1] || !strcmp(pair[0], pair[1])) {
            say_error(b, "[order] pair %u does not name two systems", i);
            refused = 1;
            break;
        }
        b->before[i] = copy_text(pair[0], strlen(pair[0]));
        b->after[i] = copy_text(pair[1], strlen(pair[1]));
        b->order_count = i + 1;
        if (!b->before[i] || !b->after[i]) {
            refused = (uint32_t)bridge_lacks(b, nullptr, "the order a deployment states");
            break;
        }
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
            say_error(b, "[order] contradicts itself: '%s' would run before "
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
static const char *conflict(const bridge_system_t *a, const bridge_system_t *b, bool *changing)
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
int schedule_build(bridge_t *b, char *why, size_t why_len)
{
    /* in room made at create, [budget] systems wide: schedule_room */
    auto in = (bridge_system_t **)b->build_in;
    uint32_t phase, i, j, k, refused = 0;
    bool changing;

    for (phase = 0; phase <= b->phase_count && !refused; phase ++) {
        const char *where = phase < b->phase_count ? b->phases[phase] : nullptr;
        uint32_t n = 0;
        matrix_t reach;
        for (i = 0; i < b->system_count; i ++) {
            if (b->systems[i]->phase == phase && !b->systems[i]->parked) in[n ++] = b->systems[i];
        }
        reach.n = n;
        reach.words = (n + 63) / 64;
        reach.bits = b->build_bits;
        memset(reach.bits, 0, ((size_t)n * reach.words + 1) * sizeof *reach.bits);

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
    }
    return (int)refused;
}

/* ---- running: a level at a time, one job per module ---- */

typedef struct {
    module_t         *m;
    bridge_system_t **list;
    uint32_t        count;
} job_t;

/* A module's systems of one level, in the order it registered them, on one thread. */
static void run_job(void *arg)
{
    job_t *j = arg;
    module_t *m = j->m;
    auto h = (bridge_header_t *)m->priv;
    uint32_t i;
    for (i = 0; i < j->count; i ++) {
        bridge_system_t *s = j->list[i];
        bridge_system_t **slots = s->observer ? m->observers : m->systems;
        backend_err_t err = {};
        int32_t status = 0;
        uint32_t slot = 0;
        while (slots[slot] != s) slot ++;
        /* validation.md: the culprit is the system that was running. One store per dispatch. */
        h->when = s->when;
        m->running = s;
        m->turn = s->turn;
        h->system = s->observer ? slot | 0x8000u : slot;
        /* what is left of it only goes down, so it is set again before every dispatch -
         * limits.md */
        if (m->work_limit > 0) m->on->api.instance_set_work_limit(m->instance, (uint64_t)m->work_limit);
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

/* budgets.md, *Nothing is allocated in a phase*: what a dispatch works in, once. */
int schedule_room(bridge_t *b)
{
    uint32_t n = b->max_systems ? b->max_systems : 1;
    b->run_jobs = calloc(n, sizeof(job_t));
    b->run_handed = calloc(n, sizeof *b->run_handed);
    b->run_lists = calloc(n, sizeof *b->run_lists);
    b->run_callers = calloc(n, 1);
    b->build_in = calloc(n, sizeof *b->build_in);
    b->build_bits = calloc((size_t)n * ((n + 63) / 64) + 1, sizeof *b->build_bits);
    return bridge_lacks(b, b->run_jobs && b->run_handed && b->run_lists && b->run_callers
        && b->build_in && b->build_bits ? b : nullptr, "what a dispatch and a schedule are worked out in");
}

void schedule_run(bridge_t *b, uint32_t phase)
{
    uint32_t n = b->system_count, level, levels = 0, i, k, ordinal = 0;
    job_t *jobs = b->run_jobs;
    void **handed = b->run_handed;
    uint8_t *callers = b->run_callers;
    auto lists = (bridge_system_t **)b->run_lists;
    if (!n) return;
    for (i = 0; i < n; i ++) {
        if (b->systems[i]->phase == phase && b->systems[i]->level >= levels) {
            levels = b->systems[i]->level + 1;
        }
    }
    if (!levels) return;

    for (level = 0; level < levels; level ++) {
        uint32_t count = 0, listed = 0;
        /* a module's systems are gathered together, so that its job's list is one run */
        for (i = 0; i < n; i ++) {
            bridge_system_t *first = b->systems[i];
            job_t *j;
            for (k = 0; k < count && jobs[k].m != first->m; k ++) {}
            if (k < count) continue;
            j = &jobs[count];
            j->m = first->m;
            j->list = &lists[listed];
            j->count = 0;
            for (k = i; k < n; k ++) {
                bridge_system_t *s = b->systems[k];
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
        pool_run(b->pool, &(pool_batch_t){ run_job, handed, callers, count });
        /* failure.md: the invocation is abandoned, and logged - here, so that the sink is
         * only ever called on the thread that asked for the phase */
        for (k = 0; k < count; k ++) {
            for (i = 0; i < jobs[k].count; i ++) {
                bridge_system_t *s = jobs[k].list[i];
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
                say_about(b, &(say_t){ s->m, s, BRIDGE_ERR }, "%s '%s' failed: %s",
                    s->observer ? "observer" : "system", s->name, s->why);
            }
        }
    }
}

/* failure.md, *The budget*: a module is stopped at the barrier after one of its systems
 * reached its number, or after it did itself, by entities it changed that could not be
 * placed. Counted in invocations and in barriers and never in time, so that the same history
 * stops the same module at the same place every run. */
void schedule_account(bridge_t *b)
{
    module_t *m;
    for (m = b->modules; m; m = m->next) {
        const bridge_system_t *s = m->reached;
        if (m->unplaced_now) m->failures ++;
        else if (m->changed_now && m->failures) m->failures --;
        m->changed_now = m->unplaced_now = 0;
        m->reached = nullptr;
        if (m->stopped || m->max_failures <= 0 || (!s && m->failures < m->max_failures)) continue;
        m->stopped = 1;
        if (s) {
            say_about(b, &(say_t){ m, nullptr, BRIDGE_ERR }, "module '%s' is stopped: its %s '%s' reached its "
                "max_system_failures of %lld", m->identity, s->observer ? "observer" : "system",
                s->name, (long long)m->max_failures);
        } else {
            say_about(b, &(say_t){ m, nullptr, BRIDGE_ERR }, "module '%s' is stopped: it reached its "
                "max_system_failures of %lld, by entities it changed that could not be placed",
                m->identity, (long long)m->max_failures);
        }
        /* fail covers a stop as it covers a load: the deployment does not run without it */
        if (m->on_error == BRIDGE_FAIL) bridge_stop(b, m->identity, "was stopped");
    }
}
