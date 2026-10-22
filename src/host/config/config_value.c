/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "config.h"

static const value_rule_t RULES[] = {
    [CHECK_FLAG] = { ALLOW_FLAG, 0, 0, "", "true or false" },
    [CHECK_COUNT] = { ALLOW_WHOLE, 1, UINT32_MAX, "",
        "a whole number, from 1" },
    [CHECK_LIMIT] = { ALLOW_WHOLE | ALLOW_WORD, 1, INT64_MAX, "none ",
        "a whole number, from 1, or \"none\"" },
    [CHECK_LEVEL] = { ALLOW_WHOLE, 1, 7, "", "a level, from 1 to 7" },
    [CHECK_NAMES] = { ALLOW_NAMES, 0, 0, "", "a list of names" },
    [CHECK_NAMES_NONE] = { ALLOW_NAMES | ALLOW_WORD, 0, 0, "none ",
        "a list of names, or \"none\"" },
    [CHECK_ERROR] = { ALLOW_WORD, 0, 0, "fail skip ",
        "\"fail\" or \"skip\"" },
    [CHECK_ARRIVE] = { ALLOW_WORD, 0, 0, "ask load leave ",
        "\"ask\", \"load\" or \"leave\"" },
    [CHECK_TEXT] = { ALLOW_TEXT, 0, 0, "", "text" },
    [CHECK_ROUNDS] = { ALLOW_WHOLE, 0, 1'000'000, "",
        "a whole number, from 0 to 1000000" },
    [CHECK_SETTING] = { ALLOW_FLAG | ALLOW_WHOLE | ALLOW_REAL | ALLOW_TEXT
        | ALLOW_EMPTY, INT64_MIN, INT64_MAX, "",
        "text, a number, true or false, or { path = \"...\" }" },
    [CHECK_PAIRS] = { ALLOW_PAIRS, 0, 0, "", "a list of pairs of systems" },
    [CHECK_AUTHOR] = { 0, 0, 0, "",
        "only a module's author states it, in its file" }
};

static bool is_word(const char *text, const char *words)
{
    size_t len = strlen(text);
    const char *at = len ? strstr(words, text) : nullptr;

    return (at && (at == words || at[-1] == ' ') && at[len] == ' ');
}

static bool is_names(const value_t *v, uint32_t count)
{
    bool names = v->kind == VALUE_LIST && (!count || v->count == count);

    for (uint32_t i = 0; names && i < v->count; i++)
        names = v->items[i].kind == VALUE_TEXT && v->items[i].text[0];
    return (names);
}

static bool is_pairs(const value_t *v)
{
    bool pairs = v->kind == VALUE_LIST && v->count;

    for (uint32_t i = 0; pairs && i < v->count; i++)
        pairs = is_names(&v->items[i], 2);
    return (pairs);
}

static bool is_allowed(const value_t *v, const value_rule_t *rule)
{
    switch (v->kind) {
        case VALUE_FLAG:
            return (rule->allow & ALLOW_FLAG);
        case VALUE_WHOLE:
            return ((rule->allow & ALLOW_WHOLE) && v->whole >= rule->least
                && v->whole <= rule->most);
        case VALUE_REAL:
            return (rule->allow & ALLOW_REAL);
        case VALUE_TEXT:
            return (((rule->allow & ALLOW_WORD) && is_word(v->text,
                rule->words)) || ((rule->allow & ALLOW_TEXT)
                && (v->text[0] || (rule->allow & ALLOW_EMPTY))));
        default:
            return (((rule->allow & ALLOW_NAMES) && is_names(v, 0))
                || ((rule->allow & ALLOW_PAIRS) && is_pairs(v)));
    }
}

const char *config_check_value(const value_t *v, enum check check)
{
    return (is_allowed(v, &RULES[check]) ? nullptr : RULES[check].why);
}
