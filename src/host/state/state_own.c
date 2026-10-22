/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static char *copy_text(const char *text, size_t size)
{
    char *copy = malloc(size + 1);

    if (copy)
        memcpy(copy, text, size);
    if (copy)
        copy[size] = 0;
    return (copy);
}

bool read_own(const handed_t *of, const char *key, value_t *v)
{
    size_t size = 0;
    const char *raw = manifest_get_value((const unsigned char *)of->bytes,
        of->len, key, &size);
    char *text = raw ? malloc(size + 8) : nullptr;
    toml_t t = {};

    if (!text)
        return (false);
    snprintf(text, size + 8, "v = %.*s\n", (int)size, raw);
    if (toml_read(text, &t) && t.count == 1)
        *v = t.entries[0].value;
    else
        *v = (value_t){ .kind = VALUE_TEXT, .text = copy_text(raw, size) };
    return (v->kind != VALUE_TEXT || v->text);
}

const value_t *state_find_own(const stating_t *s, const module_rule_t *rule)
{
    value_t *own = malloc(sizeof(*own));
    const char *why = nullptr;

    if (!own || !read_own(s->of, rule->name, own))
        return (nullptr);
    why = config_check_value(own, rule->check);
    if (why)
        printf("%sits file states %s: %s\n", s->prefix, rule->name, why);
    return (why ? nullptr : own);
}

static void say_own_keys(const stating_t *s)
{
    const manifest_t *who = &s->of->who;

    if (!s->show)
        return;
    printf("  config = \"%s\"  its file\n", who->config);
    printf("  settings = \"%s\"  its file\n", who->settings);
    printf("  pinned = %s  its file\n", who->pinned ? "true" : "false");
    printf("  reloadable = %s  its file\n",
        who->reloadable ? "true" : "false");
}

bool state_own_keys(const stating_t *s)
{
    const manifest_t *who = &s->of->who;

    if (!who->config[0])
        printf("%sits file states no config\n", s->prefix);
    if (!who->has_pinned)
        printf("%sits file states no pinned\n", s->prefix);
    if (!who->has_reloadable)
        printf("%sits file states no reloadable\n", s->prefix);
    say_own_keys(s);
    return (who->config[0] && who->has_pinned && who->has_reloadable);
}
