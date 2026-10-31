/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bits.h"
#include "schedule.h"

static uint32_t hand_word(schedule_pass_t *pass, uint32_t first, uint64_t word,
    uint32_t level)
{
    schedule_job_t *jobs = pass->w->run_jobs;
    uint32_t j;

    while (word) {
        j = first + (uint32_t)__builtin_ctzll(word);
        word &= word - 1;
        if (jobs[j].level != level)
            schedule_run_batch(pass);
        level = jobs[j].level;
        schedule_hand_job(pass, &jobs[j]);
    }
    return (level);
}

void schedule_trigger(bridge_system_t *s)
{
    bridge_world_t *w = s->m->w;

    s->next_triggered = w->triggered;
    w->triggered = s;
}

void schedule_untrigger(bridge_world_t *w, const module_t *m)
{
    bridge_system_t **at = &w->triggered;

    while (*at) {
        if ((*at)->m == m)
            *at = (*at)->next_triggered;
        else
            at = &(*at)->next_triggered;
    }
}

void schedule_clear_triggered(bridge_world_t *w)
{
    bridge_system_t *s;

    while (w->triggered) {
        s = w->triggered;
        w->triggered = s->next_triggered;
        s->next_triggered = nullptr;
        s->pending_count = 0;
    }
}

void schedule_run_triggered(schedule_pass_t *pass)
{
    bridge_world_t *w = pass->w;
    uint64_t *bits = w->build_bits;
    uint32_t words = bits_words(w->job_count);
    uint32_t level = UINT32_MAX;

    for (uint32_t i = 0; i < words; i++)
        bits[i] = 0;
    for (const bridge_system_t *s = w->triggered; s; s = s->next_triggered)
        bits_set(bits, s->job);
    for (uint32_t i = 0; i < words; i++)
        level = hand_word(pass, i * 64, bits[i], level);
    schedule_run_batch(pass);
}
