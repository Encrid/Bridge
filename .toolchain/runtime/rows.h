/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef ROWS_H
    #define ROWS_H

    #include "sweep.h"

    #ifndef _WIN32

int unsetenv(const char *name);

    #endif

constexpr int ROWS_WORDS = 64;
constexpr int ROWS_LINE = 16'384;

typedef int (*rows_run_fn_t)(int count, char **words);

static inline int rows_split(char *line, char **words)
{
    char *word = strtok(line, " \t\r\n");
    int count = 0;

    while (word && count < ROWS_WORDS - 2) {
        words[count++] = strcmp(word, "\"\"") ? word : word + 2;
        word = strtok(nullptr, " \t\r\n");
    }
    return (count);
}

static inline void rows_env(char *text, bool on)
{
    char *equals = strchr(text, '=');

    if (!on && equals)
        equals[1] = '\0';
    #ifdef _WIN32
    _putenv(text);
    #else
    if (on)
        putenv(text);
    if (!on && equals) {
        equals[0] = '\0';
        unsetenv(text);
    }
    #endif
}

static inline void rows_refuse(char *calls, bool on)
{
    char *comma = strchr(calls, ',');

    while (comma) {
        *comma = ' ';
        comma = strchr(comma, ',');
    }
    sweep_plan(on ? calls : "");
}

static inline int rows_mark(char **words, int count, bool on)
{
    int at = 0;

    while (at + 1 < count && words[at][0] == '+') {
        if (!strcmp(words[at], "+refuse"))
            rows_refuse(words[at + 1], on);
        if (!strcmp(words[at], "+env"))
            rows_env(words[at + 1], on);
        at += 2;
    }
    return (at);
}

static inline int rows_run(rows_run_fn_t run)
{
    static char line[ROWS_LINE];
    static char name[] = "row";
    char *words[ROWS_WORDS];
    char *given[ROWS_WORDS] = { name };
    int count;
    int first;

    while (fgets(line, sizeof(line), stdin)) {
        if (!strchr(line, '\n') && !feof(stdin))
            return (printf("a row is too long\n"), 1);
        count = rows_split(line, words);
        first = rows_mark(words, count, true);
        for (int i = first; i < count; i++)
            given[i - first + 1] = words[i];
        given[count - first + 1] = nullptr;
        printf("status %d\n\n", run(count - first + 1, given));
        (void)rows_mark(words, count, false);
        fflush(stdout);
    }
    return (0);
}

#endif
