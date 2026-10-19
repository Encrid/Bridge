/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "toml.h"

static size_t put_utf8(uint32_t code, char *out)
{
    static const unsigned char FIRST[] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0 };
    size_t len = 1 + (code >= 0x80) + (code >= 0x800) + (code >= 0x10000);

    for (size_t i = len - 1; i > 0; i--) {
        out[i] = (char)(0x80 | (code & 0x3F));
        code >>= 6;
    }
    out[0] = (char)(FIRST[len] | code);
    return (len);
}

static size_t read_code(reading_t *r, char *out)
{
    size_t digits = *r->at == 'u' ? 4 : 8;
    char hex[9] = "";
    uint32_t code = 0;

    r->at++;
    if (strspn(r->at, "0123456789abcdefABCDEF") < digits) {
        toml_refuse(r, "an escape with too few digits");
        return (0);
    }
    memcpy(hex, r->at, digits);
    code = (uint32_t)strtoul(hex, nullptr, 16);
    r->at += digits;
    if (code > 0x10FFFF || (code >= 0xD800 && code <= 0xDFFF)) {
        toml_refuse(r, "an escape that is no character");
        return (0);
    }
    return (put_utf8(code, out));
}

static size_t read_escape(reading_t *r, char *out)
{
    static const char FROM[] = "btnfr\"\\";
    static const char TO[] = "\b\t\n\f\r\"\\";
    const char *at = *r->at ? strchr(FROM, *r->at) : nullptr;

    if (*r->at == 'u' || *r->at == 'U')
        return (read_code(r, out));
    if (!at) {
        toml_refuse(r, "an escape that is none");
        return (0);
    }
    *out = TO[at - FROM];
    r->at++;
    return (1);
}

static size_t read_char(reading_t *r, char quote, char *out)
{
    if (*r->at == '\\' && quote == '"') {
        r->at++;
        return (read_escape(r, out));
    }
    *out = *r->at;
    r->at++;
    return (1);
}

char *toml_read_text(reading_t *r)
{
    char quote = *r->at;
    char *text = malloc(strcspn(r->at, "\n") + 1);
    size_t len = 0;
    size_t step = 1;

    if (text && r->at[1] == quote && r->at[2] == quote)
        step = toml_refuse(r, "text over lines, which is not taken");
    r->at += step;
    while (text && step && *r->at && *r->at != quote && *r->at != '\n') {
        step = read_char(r, quote, text + len);
        len += step;
    }
    if (!text || !step || *r->at != quote) {
        toml_refuse(r, "text with no end");
        free(text);
        return (nullptr);
    }
    r->at++;
    text[len] = 0;
    return (text);
}
