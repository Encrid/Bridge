/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "manifest_inside.h"

static const char *read_word(manifest_t *m, char *word, bool first)
{
    static char why[160];
    char *value = strchr(word, '=');

    if (first && (!strchr(word, ':') || !strchr(word, '@') || value))
        return ("its manifest does not begin with who it is");
    if (first) {
        strcpy(m->identity, word);
        return (nullptr);
    }
    if (value)
        *value++ = 0;
    if (value && manifest_take(m, word, value) >= 0)
        return (nullptr);
    snprintf(why, sizeof(why), "states a %.60s that cannot be", word);
    return (why);
}

static const char *parse(const char *text, size_t size, manifest_t *m)
{
    char word[160];
    const char *why = nullptr;
    size_t at = 0;
    size_t len;

    while (!why && at < size && text[at]) {
        len = strnlen(text + at, size - at);
        if (len >= sizeof(word) || (!at && len >= sizeof(m->identity)))
            return ("its manifest holds a word that is too long");
        memcpy(word, text + at, len);
        word[len] = 0;
        why = read_word(m, word, !at);
        at += len + 1;
    }
    if (why)
        return (why);
    if (!m->identity[0])
        return ("its manifest is empty");
    return (m->abi ? nullptr : "states no abi");
}

const char *manifest_read(const unsigned char *bytes, size_t len,
    manifest_t *m)
{
    size_t at = 0;
    size_t size = 0;

    *m = (manifest_t){};
    m->backend = manifest_find(bytes, len, &at, &size);
    if (!m->backend)
        return ("neither a wasm module nor a library");
    if (!size)
        return ("no manifest");
    return (parse((const char *)bytes + at, size, m));
}

const char *manifest_get_value(const unsigned char *bytes, size_t len,
    const char *key, size_t *size)
{
    size_t name = strlen(key);
    size_t at = 0;
    size_t left = 0;
    size_t word;

    if (!manifest_find(bytes, len, &at, &left))
        return (nullptr);
    while (left && bytes[at]) {
        word = strnlen((const char *)bytes + at, left);
        if (word > name && bytes[at + name] == '='
            && !memcmp(bytes + at, key, name)) {
            *size = word - name - 1;
            return ((const char *)bytes + at + name + 1);
        }
        at += word + 1;
        left -= word < left ? word + 1 : left;
    }
    return (nullptr);
}
