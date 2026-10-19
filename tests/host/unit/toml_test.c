/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "toml.h"

static int fails;

static void ck(bool cond, const char *what)
{
    if (!cond) {
        printf("unit FAIL: %s\n", what);
        fails++;
    }
}

static void try_ok(const char *text)
{
    toml_t t = {};

    ck(toml_read(text, &t), text);
}

static void try_no(const char *text, const char *why)
{
    toml_t t = {};
    bool taken = toml_read(text, &t);

    ck(!taken, text);
    ck(!taken && t.why && strstr(t.why, why), why);
}

static void do_scalars(void)
{
    try_ok("\xEF\xBB\xBF" "a = 1\n");
    try_ok("a = 1\n");
    try_ok("a = 1");
    try_ok("# comment\n");
    try_ok("\n");
    try_ok("   ");
    try_ok("a = 1\r\n");
    try_no("a = 1\r", "line");
    try_ok("a = 1  \n");
    try_ok("a\t= 1\n");
    try_ok("a = \"x\" # after\n");
    try_ok("a = true\nb = false\n");
    try_ok("a = 'single'\n");
    try_ok("Key = 1\n");
    try_ok("a1 = 1\n");
    try_ok("a-b = 1\n");
    try_ok("'quoted key' = 1\n");
    try_ok("a.b.c = 1\n");
    try_ok("a = 1\nb = 2\n");
    try_ok("[t]\nx = 1\n");
}

static void do_numbers(void)
{
    try_ok("a = 0\n");
    try_ok("a = 10\n");
    try_ok("a = +0\n");
    try_ok("a = -5\n");
    try_ok("a = 1_000\n");
    try_ok("a = 1.5\n");
    try_ok("a = 1e3\n");
    try_ok("a = 9223372036854775807\n");
    try_no("a = 1_\n", "number");
    try_no("a = +_1\n", "number");
    try_no("a = 01\n", "number");
    try_no("a = 12:30\n", "number");
    try_no("a = 12x\n", "number");
    try_no("a = +\n", "number");
    try_no("a = 1-2\n", "number");
    try_no("a = 99999999999999999999999\n", "number");
    try_no("a = 1.\n", "number");
    try_no("a = 1.2.3\n", "number");
    try_no("a = +e5\n", "number");
    try_no("a = 1111111111111111111111111111111111111111"
        "11111111111111111111111111\n", "number");
}

static void do_text(void)
{
    try_ok("a = \"\"\n");
    try_ok("a = \"d\\u00e9f\"\n");
    try_ok("a = \"\\u0041\"\n");
    try_ok("a = \"\\U0001F600\"\n");
    try_ok("a = \"tab\\there\"\n");
    try_ok("a = \"d\\\"q\"\n");
    try_ok("a = \"\\uE000\"\n");
    try_ok("a = 'x\\y'\n");
    try_no("a = \"\\uxy\"\n", "escape");
    try_no("a = \"\\U00110000\"\n", "character");
    try_no("a = \"\\uD800\"\n", "character");
    try_no("a = \"\\q\"\n", "escape");
    try_no("a = \"x\\", "escape");
    try_no("a = \"\"\"multi\n", "lines");
    try_no("a = \"open\n", "end");
    try_no("a = \"noclose", "end");
}

static void do_structure(void)
{
    try_ok("a = [1, 2, 3]\n");
    try_ok("a = []\n");
    try_ok("a = [\n1,\n# c\n2\n]\n");
    try_ok("a = [1,\r\n2]\n");
    try_ok("a = { x = 1, y = 2 }\n");
    try_ok("a = { x = { y = 1 } }\n");
    try_no("a = [1 2]\n", "list");
    try_no("a = [@]\n", "none");
    try_no("a = [ {x=1} ]\n", "list");
    try_no("a = { x }\n", "=");
    try_no("a = { = 1 }\n", "name");
    try_no("a = { x = @ }\n", "none");
    try_no("a = { x = 1", "}");
    try_no("a = { x=1, x=2 }\n", "twice");
}

static void do_refusals(void)
{
    try_no("[[t]]\n", "array");
    try_no("[]\n", "name");
    try_no("[t\n", "]");
    try_no("a = 1\n[a.b]\n", "value and a table");
    try_no("a = 1\na = 2\n", "twice");
    try_no("a = 1\nb = 2\na = 3\n", "twice");
    try_no("a.b.c = 1\na.b = 2\n", "value and a table");
    try_no("a~b = 1\n", "=");
    try_no("a 1\n", "=");
    try_no("= 1\n", "name");
    try_no("a = @\n", "none");
    try_no("a = fnord\n", "none");
    try_no("a = tru\n", "none");
    try_no("a = :\n", "none");
    try_no("a = !\n", "none");
    try_no("a = 1 b\n", "line");
}

static void do_same(void)
{
    value_t two[2] = { { .kind = VALUE_WHOLE, .whole = 5 },
        { .kind = VALUE_WHOLE, .whole = 6 } };
    value_t three[2] = { { .kind = VALUE_WHOLE, .whole = 5 },
        { .kind = VALUE_WHOLE, .whole = 7 } };
    value_t la = { .kind = VALUE_LIST, .items = two, .count = 2 };
    value_t lb = { .kind = VALUE_LIST, .items = two, .count = 2 };
    value_t lc = { .kind = VALUE_LIST, .items = three, .count = 2 };
    value_t ld = { .kind = VALUE_LIST, .items = two, .count = 1 };

    ck(!toml_is_same(&(value_t){ .kind = VALUE_WHOLE },
        &(value_t){ .kind = VALUE_TEXT, .text = (char *)"x" }), "same kind");
    ck(toml_is_same(&(value_t){ .kind = VALUE_TEXT, .text = (char *)"x" },
        &(value_t){ .kind = VALUE_TEXT, .text = (char *)"x" }), "same text");
    ck(!toml_is_same(&(value_t){ .kind = VALUE_TEXT, .text = (char *)"x" },
        &(value_t){ .kind = VALUE_TEXT, .text = (char *)"y" }), "diff text");
    ck(!toml_is_same(&(value_t){ .kind = VALUE_FLAG, .flag = true },
        &(value_t){ .kind = VALUE_FLAG, .flag = false }), "diff flag");
    ck(toml_is_same(&two[0], &(value_t){ .kind = VALUE_WHOLE, .whole = 5 }),
        "same whole");
    ck(!toml_is_same(&two[0], &two[1]), "diff whole");
    ck(toml_is_same(&(value_t){ .kind = VALUE_REAL, .real = 1.5 },
        &(value_t){ .kind = VALUE_REAL, .real = 1.5 }), "same real");
    ck(!toml_is_same(&(value_t){ .kind = VALUE_REAL, .real = 1.5 },
        &(value_t){ .kind = VALUE_REAL, .real = 2.5 }), "diff real");
    ck(toml_is_same(&la, &lb), "same list");
    ck(!toml_is_same(&la, &lc), "diff item");
    ck(!toml_is_same(&la, &ld), "diff count");
}

static void do_say(void)
{
    value_t items[3] = { { .kind = VALUE_WHOLE, .whole = 100 },
        { .kind = VALUE_WHOLE, .whole = 200 },
        { .kind = VALUE_WHOLE, .whole = 300 } };
    value_t list = { .kind = VALUE_LIST, .items = items, .count = 3 };
    char big[64];
    char tiny[4];

    toml_say(&(value_t){ .kind = VALUE_TEXT, .text = (char *)"hi" }, big,
        sizeof(big));
    ck(!strcmp(big, "\"hi\""), "say text");
    toml_say(&(value_t){ .kind = VALUE_WHOLE, .whole = 42 }, big, sizeof(big));
    ck(!strcmp(big, "42"), "say whole");
    toml_say(&(value_t){ .kind = VALUE_REAL, .real = 3.5 }, big, sizeof(big));
    ck(!strcmp(big, "3.5"), "say real");
    toml_say(&(value_t){ .kind = VALUE_FLAG, .flag = true }, big, sizeof(big));
    ck(!strcmp(big, "true"), "say flag");
    toml_say(&list, big, sizeof(big));
    ck(!strcmp(big, "[100, 200, 300]"), "say list");
    toml_say(&list, tiny, sizeof(tiny));
    ck(tiny[0] == '[', "say tiny");
}

static void do_prefix(void)
{
    char *shallow[1] = { (char *)"a" };
    char *deep[2] = { (char *)"a", (char *)"b" };
    const char *const names[2] = { "a", "b" };
    entry_t one = { .key = shallow, .depth = 1 };
    entry_t two = { .key = deep, .depth = 2 };

    ck(!toml_has_prefix(&one, names, 2), "prefix deep");
    ck(toml_has_prefix(&two, names, 2), "prefix match");
    ck(!toml_has_prefix(&two, (const char *const[]){ "a", "c" }, 2),
        "prefix miss");
}

static void value_whole(void)
{
    value_t five = { .kind = VALUE_WHOLE, .whole = 5 };
    value_t zero = { .kind = VALUE_WHOLE, .whole = 0 };
    value_t big = { .kind = VALUE_WHOLE, .whole = 5000000000 };
    value_t flag = { .kind = VALUE_FLAG, .flag = true };
    value_t real = { .kind = VALUE_REAL, .real = 1.5 };

    ck(!config_check_value(&five, CHECK_COUNT), "count ok");
    ck(config_check_value(&zero, CHECK_COUNT), "count low");
    ck(config_check_value(&big, CHECK_COUNT), "count high");
    ck(config_check_value(&flag, CHECK_COUNT), "count flag");
    ck(!config_check_value(&flag, CHECK_FLAG), "flag ok");
    ck(config_check_value(&five, CHECK_FLAG), "flag whole");
    ck(config_check_value(&real, CHECK_COUNT), "real no");
    ck(!config_check_value(&real, CHECK_SETTING), "real ok");
    ck(config_check_value(&zero, CHECK_LEVEL), "level low");
    ck(config_check_value(&(value_t){ .kind = VALUE_WHOLE, .whole = 8 },
        CHECK_LEVEL), "level high");
}

static void value_text(void)
{
    value_t word = { .kind = VALUE_TEXT, .text = (char *)"x" };
    value_t empty = { .kind = VALUE_TEXT, .text = (char *)"" };

    ck(!config_check_value(&word, CHECK_TEXT), "text ok");
    ck(config_check_value(&empty, CHECK_TEXT), "text empty");
    ck(!config_check_value(&empty, CHECK_SETTING), "setting empty");
    ck(!config_check_value(&(value_t){ .kind = VALUE_TEXT,
        .text = (char *)"none" }, CHECK_LIMIT), "limit word");
    ck(config_check_value(&(value_t){ .kind = VALUE_TEXT,
        .text = (char *)"other" }, CHECK_LIMIT), "limit other");
    ck(config_check_value(&(value_t){ .kind = VALUE_TEXT,
        .text = (char *)"ail" }, CHECK_ERROR), "error ail");
    ck(config_check_value(&(value_t){ .kind = VALUE_TEXT,
        .text = (char *)"fai" }, CHECK_ERROR), "error fai");
    ck(config_check_value(&empty, CHECK_ERROR), "error empty");
    ck(config_check_value(&word, CHECK_ERROR), "error word");
    ck(!config_check_value(&(value_t){ .kind = VALUE_TEXT,
        .text = (char *)"fail" }, CHECK_ERROR), "error fail");
    ck(config_check_value(&(value_t){ .kind = VALUE_FLAG, .flag = true },
        CHECK_AUTHOR), "author");
}

static void value_list(void)
{
    value_t names[2] = { { .kind = VALUE_TEXT, .text = (char *)"a" },
        { .kind = VALUE_TEXT, .text = (char *)"b" } };
    value_t bad[1] = { { .kind = VALUE_WHOLE, .whole = 1 } };
    value_t miss[2] = { { .kind = VALUE_TEXT, .text = (char *)"a" },
        { .kind = VALUE_TEXT, .text = (char *)"" } };
    value_t good = { .kind = VALUE_LIST, .items = names, .count = 2 };
    value_t none = { .kind = VALUE_LIST, .items = bad, .count = 1 };
    value_t gap = { .kind = VALUE_LIST, .items = miss, .count = 2 };
    value_t pair = { .kind = VALUE_LIST, .items = &good, .count = 1 };
    value_t trio[3] = { names[0], names[1], names[0] };
    value_t wide = { .kind = VALUE_LIST, .items = trio, .count = 3 };
    value_t pair3 = { .kind = VALUE_LIST, .items = &wide, .count = 1 };
    value_t empty = { .kind = VALUE_LIST };
    value_t nothing = {};

    ck(!config_check_value(&good, CHECK_NAMES), "names ok");
    ck(config_check_value(&none, CHECK_NAMES), "names whole");
    ck(config_check_value(&gap, CHECK_NAMES), "names empty");
    ck(config_check_value(&(value_t){ .kind = VALUE_WHOLE, .whole = 1 },
        CHECK_NAMES), "names scalar");
    ck(!config_check_value(&pair, CHECK_PAIRS), "pairs ok");
    ck(config_check_value(&empty, CHECK_PAIRS), "pairs empty");
    ck(config_check_value(&none, CHECK_PAIRS), "pairs scalars");
    ck(config_check_value(&pair3, CHECK_PAIRS), "pairs trio");
    ck(config_check_value(&nothing, CHECK_PAIRS), "pairs none");
    ck(config_check_value(&good, CHECK_PAIRS), "pairs names");
    ck(!config_check_value(&good, CHECK_NAMES_NONE), "names none list");
}

int main(int argc, char **argv)
{
    (void)argv;
    setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc > 1)
        return (1);
    do_scalars();
    do_numbers();
    do_text();
    do_structure();
    do_refusals();
    do_same();
    do_say();
    do_prefix();
    value_whole();
    value_text();
    value_list();
    printf("%s\n", fails ? "unit broken" : "unit ok");
    return (fails ? 1 : 0);
}
