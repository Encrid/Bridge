# .toolchain/runtime/rows.h

`rows.h` turns a test program that reads its arguments into a program of a table: `docs/contributing/tests.md`, *Tables*.
The program gives its old entry to `rows_run`, and `rows_run` calls it one time for each line of the input.
The artifact of the program gets `rows.h` as it gets `sweep.h`: `.toolchain/docs/sweep.h.md`.

```c
static int run(int count, char **words)
{
    return (count > 1 && !strcmp(words[1], "greet") ? greet(words[2]) : 2);
}

int main(void)
{
    return (rows_run(run));
}
```

## A row

A line of the input is one row. `rows_run` cuts it into words at each space and tab.

- The words are the arguments of the entry: `words[0]` is `row`, and `words[1]` is the first word of the line, as for `main`.
- The word `""` is an empty argument.
- After the entry returns, `rows_run` writes `status <result>`, and then one empty line. Thus a row claims its status as a line: `status 0`.
- The entry starts each row from a clean state: it sets again each variable that a row before it changed.
- An entry that calls `exit` ends the program, and each row after it has no answer. Such a behavior needs a case of its own.

## The marks of a row

A row can start with marks. A mark begins with `+`, and `rows_run` does not give it to the entry.

| Mark | Effect |
| --- | --- |
| `+refuse <function>=<number>` | In this row, the call number `<number>` of the function fails, as with `sweep_plan`: `.toolchain/docs/sweep.h.md`. The case states the function in `sweep`. After the row, no call fails. |
| `+env <name>=<value>` | In this row, the variable of the environment has the value. After the row, `rows_run` removes the variable. |

The mark `+refuse` takes more than one function, with a comma between them: `+refuse malloc=1,write=1`.
A system that has no call of a function of the list refuses no call of it.

A row has 61 words at most, and a line 16383 characters at most. For a longer line, the program writes `a row is too long`, and ends with the status 1.

## Functions

| Function | Result |
| --- | --- |
| `rows_split(line, words)` | Cuts a line into its words, and gives their count. |
| `rows_env(text, on)` | Sets the variable of the text `<name>=<value>` in the environment, or removes it. |
| `rows_refuse(calls, on)` | Makes the calls of the list fail with `sweep_plan`, or makes no call fail. |
| `rows_mark(words, count, on)` | Does the marks at the start of a row, or undoes them. It gives the place of the first word after them. |
| `rows_run(run)` | Reads each line of the input, and calls the entry for it. It gives 0. |
