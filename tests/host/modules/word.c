/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "word.h"

static uint32_t word;
static bool said;

BRIDGE_EXPORT(say) int say(void)
{
    char text[sizeof(((word_t *)0)->word)];
    uint32_t len = 0;

    if (!bridge_has(word, 0) || said)
        return (0);
    while (len < sizeof(text)
        && acc_ld_u8(word, BRIDGE_AT(0, 0),
            (uint32_t)offsetof(word_t, word) + len)) {
        text[len] = (char)acc_ld_u8(word, BRIDGE_AT(0, 0),
            (uint32_t)offsetof(word_t, word) + len);
        len++;
    }
    bridge_log(BRIDGE_LOG_INFO, &(bridge_line_t){ text, len, 0, 0 });
    said = true;
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};

    word = BRIDGE_COMPONENT("bridge-host:Word", word_t, BRIDGE_DELETE,
        BRIDGE_SINGLETON);
    terms[0].slot = word;
    BRIDGE_ADD_SYSTEM("say", "late", terms, 1, nullptr, 0, nullptr, 0);
    return (0);
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
    return (0);
}
