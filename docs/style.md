# Coding style

How Bridge-ASM700's C is written. Why the tree has the shape it has is `design.md`; which C it is
written in is `build.md`, *The standard*.

The coding style is a set of rules and conventions for:

- the organization of the repository,
- the naming of identifiers,
- the overall presentation of a file,
- the presentation of a line,
- source files and headers.

It is a purely syntactic convention. It cannot make a program work, and following it is no
excuse for one that does not.

It applies to every C source file (`.c`) and header (`.h`) that is Bridge-ASM700's own: the
library, the seam headers, the backends and their SDKs, the test programs and the test
modules. It does not apply to what is somebody else's and only carried here, such as a
backend's dependency.

It is easier and quicker to follow the style from the first line than to bend finished code to
it afterwards.

The style exists so that every file reads as if one person had written it, which makes easier:

- its reading,
- its debugging,
- its maintenance,
- its internal logic,
- its reuse,
- writing tests,
- adding new features.

When a choice has to be made and no rule decides it, ask which option makes the code clearer,
more ergonomic and more flexible.

**A rule may be infringed where a complete, relevant and accurate justification is given**, one
that takes the long view: cleanliness, legibility, flexibility, speed that was measured. Such a
justification is not left in a comment beside the code. It is written once, in *Exceptions*,
below, naming the rule and what is excepted, and an infringement that is not there is a defect.

There are many ways to write unclean code, and this document cannot name them all. A way that
is not named here is still not welcome: it is an implicit rule.

This document is the Epitech C Coding Style, version 3.1, taken as it is and made this
project's own. That style is inspired by the Linux Kernel Coding Style and freely adapted from
Robert C. Martin's book Clean Code. What differs here from the original is the file header of
rule G1, the form rule F6 gives the comment above a function, and the two sections at the end,
which the original does not have.

`tools/style.py` holds every rule here that a program can hold, and runs with the tests.

## O - Files organization

### O1 - Contents of the repository

The repository holds only the files that are needed to build it, test it and understand it.

Nothing a build makes is tracked: no compiled file (`.o`, `.gch`, `.a`, `.so`, `.dll`, ...),
and no temporary or unnecessary file (`*~`, `#*#`, `*.d`, `a.out`, ...). What a build makes
goes under `build/`, which is not tracked.

A library somebody else built and published is not something a build of ours makes. It is
tracked as it was published, in a folder of its own that holds its licence.

### O2 - File extension

Sources in C have the extension `.c` or `.h` and no other.

### O3 - File coherence

A source file matches one logical entity, and groups all the functions that belong to that
entity.

Beyond 5 functions in a file, the logical entity is to be divided into several sub-entities,
each with a file of its own.

### O4 - Naming files and folders

The name of a file says which logical entity it holds, and so is clear, precise, explicit and
unambiguous.

For example, `string.c` and `algo.c` are probably badly named. `string_toolbox.c` and
`pathfinding.c` would be better.

Every file name and folder name is in English and in snake_case: lowercase letters, digits and
underscores, and nothing else.

**That is every file and folder that is tracked**, a document and a transcript as much as a
source: `host_api.md`, `stage12.md`, `storage_sparse.txt`. Two kinds of name are not ours to
choose: one a tool asks for, which begins with a dot, and what is inside a folder somebody else
published.

Abbreviations are tolerated where they shorten the name by much and lose none of its meaning.

## G - Global scope

### G1 - File header

Every source file (`.c`, `.h`) starts with the project's standard header.

```c
/*
** Copyright (C) $YEAR Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** $PATH_OF_THE_FILE -- what the file is
*/
```

`$YEAR` is the year the file was first written. `$PATH_OF_THE_FILE` is its path from the root
of the repository, and what follows it says what the file is, in as many lines as are relevant
and no more.

```c
/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage.c -- what keeps the rows: tables, and a barrier's work
*/
```

**A file that carries somebody else's code names them**, each on a line of its own under ours,
with where the code is from and its licence:

```c
/*
** Copyright (C) 2026 Heuretes-Workshop
** Copyright (C) 2005-2020 Rich Felker, et al. (musl, MIT)
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/text.c -- strtoll and strtoull, after musl's
*/
```

### G2 - Separation of functions

Inside a source file, one empty line and only one separates the implementations of two
functions.

### G3 - Indentation of pre-processor directives

Pre-processor directives are indented according to their level of indirection.

```c
#ifndef WIN32
    #if defined(__i386__) || defined(__x86_64__)
        const size_t PAGE_SIZE = 4096;
    #else
        #error "Unknown architecture"
    #endif
#endif
```

Rule L2 applies to pre-processor directives as well.

### G4 - Global variables

Any non-trivial constant is defined by a constant global variable.

Global variables that are not constant are to be avoided as much as possible.

A variable is constant if and only if it is correctly marked as such, with the `const` keyword
or with `constexpr`. Watch out: `const` follows some particular and sometimes surprising rules.

```c
const float GOLDEN_RATIO = 1.61803398875;       /* OK */
```

### G5 - Static

Global variables and functions that are not used outside the compilation unit they belong to
are marked with the `static` keyword.

Be careful not to confuse the different uses of the `static` keyword.

## F - Functions

### F1 - Coherence of functions

A function does one thing only. It does not mix levels of abstraction, and it respects the
principle of single responsibility: a function is changed for one reason only.

For example, a call to `malloc()`, a call to `allocate_user()` and a call to `create_user()`
are at three different levels of abstraction.

### F2 - Naming functions

The name of a function says what task it carries out, and contains a verb.

For example, `voyals_nb()` and `dijkstra()` are badly named. `get_voyals_number()` and
`search_shortest_path()` are more meaningful and more precise.

Every function name is in English and in snake_case: lowercase letters, digits and
underscores, and nothing else.

**The verb says whether the call changes anything.** A function that answers yes or no begins
with `is_` or `has_`. A function that answers something it holds, and changes nothing, begins
with `get_`. Every other function is named by the verb of what it does. A name that begins with
what it belongs to keeps that first: `storage_is_alive()`, `bridge_get_rows()`.

```c
bool has_moved(const storage_t *r, uint64_t id);            /* OK */
bool moved(const storage_t *r, uint64_t id);                /* F2 violation */
set_t *get_set(storage_t *r, uint32_t slot);                /* OK */
set_t *set_of(storage_t *r, uint32_t slot);                 /* F2 violation */
uint64_t hash_type(const type_t *type);                     /* OK */
uint64_t fingerprint(const type_t *type);                   /* F2 violation */
```

Abbreviations are tolerated where they shorten the name by much and lose none of its meaning.

### F3 - Number of columns

A line is no longer than 80 columns. Columns are not characters: a tab is one character and
several columns.

### F4 - Number of lines

The body of a function is as short as possible.

If the body of a function is longer than 20 lines, it probably does too many tasks.

```c
int main(void)
{                               /* this function is 2 lines long */
    printf("hello, world\n");
    return (0);
}
```

> The maximum length of a function is inversely proportional to the complexity and indentation
> level of that function. So, if you have a conceptually simple function that is just one long
> (but simple) case-statement, where you have to do lots of small things for a lot of different
> cases, it's OK to have a longer function.
>
> Linus Torvalds, Linux Kernel Coding Style

### F5 - Arguments

The arguments of a function are stated as ISO/ANSI C states them.

A function that takes no parameter takes `void` as its argument in its declaration.

```c
phys_addr_t alloc_frame();          /* F5 violation */
phys_addr_t alloc_frame(void);      /* OK */
```

A function does not need more than 4 arguments.

Writing a variadic function is allowed, and is not a way around the limit of 4 parameters.

A structure is passed as a parameter through a pointer, and not by copy.

```c
void make_some_coffee(struct my_struct *board, int i)       /* OK */
{
    do_something();
}
```

### F6 - Comments inside a function

There is no comment inside a function.

A function is readable and explains itself, with no need for more.

The length of a function being inversely proportional to its complexity, a complicated
function is a short one: a comment above it is enough to explain it.

That comment has the form of the file's header: the function's name, an empty line, and then
what it does, what it answers and what it answers when it cannot.

**A function that is not `static` always has its comment.** A `static` one has it when its
name and its arguments do not say what it does.

**A document is cited the way the documents cite each other**, the file and then the heading
between asterisks, on the last lines of the comment after an empty one. `tools/xref.py` reads
them, so a heading that is renamed does not leave a function pointing at nothing.

```c
/*
** storage_apply
**
** Applies every module's queue, in the order the calls that filled them ran.
** An entity that cannot be placed is left as it was.
*/
void storage_apply(storage_t *r)
{
    ...
}
```

## L - Layout inside a function scope

### L1 - Code line content

A line holds one statement only.

Typical situations to avoid are:

- several assignments on one line,
- several semicolons on one line, used to separate several sequences of code,
- a condition and an assignment on one line.

```c
a = b = c = 0;                                          /* L1 violation */
a++; b++;                                               /* L1 violation */
if ((ptr = malloc(sizeof(struct my_struct))) != NULL)   /* L1 violation */
if (cond) return (ptr);                                 /* L1 violation */
a = do_something(), 5;                                  /* L1 violation */
```

### L2 - Indentation

Each level of indentation is 4 spaces.

No tabulation is used for indentation.

A `case` is one level inside its `switch`, and what it runs is one level inside the `case`.

```c
switch (kind) {
    case SCHEMA_BOOL:           /* OK */
        return (1);
    default:
        return (0);
}
```

### L3 - Spaces

Always place a space after a comma, and after a keyword if it has arguments.

There is no space between the name of a function and the opening parenthesis, nor after a unary
operator.

Every binary and ternary operator is separated from its arguments by a space on both sides.

`return` is a keyword; `sizeof` is a unary operator.

```c
return (1);                     /* OK */
return(1);                      /* L3 violation */
break;                          /* OK */
sum = term1 + 2 * term2;        /* OK */
s = sizeof(struct file);        /* OK */
i++;                            /* OK */
i ++;                           /* L3 violation */
```

### L4 - Curly brackets

An opening curly bracket is at the end of its line, except for a function, where it is alone on
its line.

A closing curly bracket is always alone on its line, except before an `else` statement.

For a scope of a single line, curly brackets are optional.

```c
if (cond) {return (ptr);}       /* L1 & L4 violation */
while (cond) {                  /* OK */
    do_something();
}
if (cond)
{                               /* L4 violation */
    ...
} else {                        /* OK */
    ...
}
if (cond)                       /* OK */
    return (ptr);
int print_env(void)
{                               /* OK */
    ...
}
int print_env(void) {           /* L4 violation */
    ...
}
```

### L5 - Variable declaration

Variables are declared at the beginning of the scope of the function.

The counter of a `for` loop may be declared within the loop.

Nothing prevents a variable from being declared and given its value on the same line.

Only one variable is declared a line.

```c
long calculate_gcd(long a, long b)
{
    long biggest, smallest;             /* L5 violation */

    biggest = MAX(a, b);
    smallest = MIN(a, b);
    long rest;                          /* L5 violation */
    while (smallest > 0) {
        rest = biggest % smallest;
        biggest = smallest;
        smallest = rest;
    }
    return (a);
}

int main(void)
{
    int forty_two = 42;                 /* OK */
    int max = 12;                       /* OK */

    for (int i = 0; i < max; i++)       /* OK */
        calculate_gcd(forty_two, max);
    return (0);
}
```

### L6 - Line jumps

One empty line separates the variable declarations from the remainder of the function.

No other empty line is present in the scope of a function.

```c
int sys_open(char const *path)
{
    int fd = thread_reserve_fd();
    struct filehandler *fhandler = NULL;
                                        /* OK */
    if (fd < 0)
        return (-1);
    if (fs_open(path, &fhandler)) {
        thread_free_fd(fd);
        return (-1);
    }
                                        /* L6 violation */
    thread_set_fd_handler(fd, fhandler);
    return (fd);
}
```

## V - Variables and types

### V1 - Naming identifiers

Every identifier name is in English and in snake_case: lowercase letters, digits and
underscores, and nothing else.

Abbreviations are tolerated where they shorten the name by much and lose none of its meaning.

The name of a type defined with `typedef` ends with `_t`.

The names of macros and of global constants, and the content of enums, are written in
UPPER_CASE.

```c
#define IS_PAGE_ALIGNED(x) (!((x) & (PAGE_SIZE - 1)))       /* OK */

enum arch {                                                 /* OK */
    I386 = 0,
    X86_64,
    ARM,
    ARM64,
    SPARC,
    POWERPC,
};

const float PI = 3.14159;                                   /* OK */
typedef int age;                                            /* V1 violation */
typedef struct int_couple pixel_t;                          /* OK */
```

### V2 - Structures

Variables are grouped together into a structure if and only if they form a coherent entity.

A structure is kept as small as possible.

```c
struct person {                         /* OK */
    char *name;
    unsigned int age;
    float salary;
};

struct trashy {                         /* V2 violation */
    struct person player;
    unsigned int width;
    unsigned int length;
    unsigned int score;
    int i;
};
```

### V3 - Pointers

The pointer symbol (`*`) is attached to the variable it belongs to, with no space between.

This rule applies in the pointer context only.

```c
int* a;                                 /* V3 violation */
int *a;                                 /* OK */
int a = 3 * b;                          /* OK */
int strlen(char const *str);            /* OK */
```

## C - Control structure

Unless a rule says otherwise, every control structure is allowed.

### C1 - Conditional branching

An `if` block does not contain more than 3 branchings, error handling excluded.

Use an array of pointers to function, or a `switch`, in its place. Take care to choose the
more suitable one: it may have to be justified.

Nested conditional branchings with a depth of 3 or more are to be avoided.

Where several levels of branching are needed, the function probably needs to be divided into
sub-functions.

```c
if (...)                                /* OK */
    do_something();
else if (...)
    do_something_else();
else
    do_something_more();

if (...)                                /* C1 violation */
    do_something();
else if (...)
    do_something_else();
else if (...)
    do_something_more();
else
    do_one_last_thing();

while (...)                             /* OK */
    if (...)
        do_something();

while (...)                             /* C1 violation */
    for (...)
        if (...)
            do_something();
```

Where this rule has to be broken, the cyclomatic complexity of the function is what would
justify it.

### C2 - Ternary

Ternaries are allowed as long as they are kept simple and readable, and do not obfuscate the
code.

Never use nested or chained ternaries.

A ternary is not used to control the flow of the program.

```c
parity_t year_parity = (year % 2 == 0) ? EVEN : ODD;        /* OK */
return (a > 0 ? a : 0);                                     /* OK */
int a = b > 10 ? c < 20 ? 50 : 80 : e == 2 ? 4 : 8;         /* C2 violation */
already_checked ? go_there() : check();                     /* C2 violation */
```

### C3 - Goto

The code does not contain the `goto` keyword, above all because it very quickly takes part in
making spaghetti code, which is completely illegible.

In rare cases its use brings readability or speed to the program, error management for
example. As always, a justification is required: *Exceptions*.

## A - Advanced

### A1 - Constant pointers

When a pointer is created, as a parameter or as a variable inside a function, and the data it
points to is not modified by the function, it is marked constant (`const`).

### A2 - Typing

Prefer the most accurate type there is for what the data is used for.

```c
int counter;                                        /* A2 violation */
unsigned int counter;                               /* OK */
unsigned int get_obj_size(void const *object);      /* A2 violation */
size_t get_obj_size(void const *object);            /* OK */
```

`ptrdiff_t`, `uint8_t`, `int32_t`, and the rest of them.

## H - Header files

### H1 - Content

A header file contains only:

- function prototypes,
- type declarations,
- global variable declarations,
- macros,
- static inline functions.

And all of these are found in header files only.

A header file may include other headers, if and only if it is necessary.

### H2 - Include guard

A header is protected from double inclusion.

The method and the conventions used are left free.

### H3 - Macros

A macro is not used for a constant.

A macro matches one statement only.

A macro that takes nothing may name what stands between the braces of a type, its fields or its
values, so that a type and the text that describes it are made of the same text -
`schema.md`, *There is no schema format*.

```c
#define PI 3.14159265358979323846                           /* H3 violation */
const float PI = 3.14159265358979323846;                    /* OK */
#define LOOP_COUNTER 0                                      /* H3 violation */
#define DELTA(a, b, c) ((b) * (b) - 4 * (a) * (c))          /* OK */
#define PRINT_NEXT(num) {num++; printf("%d", num);}         /* H3 violation */
#define POSITION_FIELDS { float x; float y; }               /* OK */
```

## What C23 adds

The rules above were written for C as it was. Bridge-ASM700 is written in C23, and uses it
fully - `build.md`, *The standard*. None of what C23 adds contradicts a rule here, and where it
gives a rule a better spelling, that spelling is the one used:

    G4, H3     a constant with a name is a typed constexpr, which is a constant global
               variable in the sense of G4, and is no macro
    L1, L6     the examples above write NULL, as their original does. Ours is nullptr
    V1         an enum is named, with its width stated: enum bridge_kind : uint32_t
    H1         a function's attributes are written in the brackets C23 gives them:
               [[nodiscard]], [[maybe_unused]]

**One spelling of C23 gives way to a rule.** C23 lets a function that takes nothing be declared
`f()`. Rule F5 asks for `f(void)`, and F5 is what is written.

## Exceptions

A rule that is infringed, with the justification the introduction asks for. Each names the rule,
what is excepted, and why. `tools/style.py` reads this section, and reports anything that is
not in it.

An exception is one line indented by four spaces: the rule, the path it is for, and the reason.
The path may hold `*`, and may end in `:` and a function's name when one function is what is
excepted. A reason that needs more lines continues on lines indented further. An exception that
excuses nothing is reported, so that none outlives what it was for.

None.
