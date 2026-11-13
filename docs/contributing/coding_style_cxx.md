# Coding style of C++

This document gives the style of the C++23 code of a project.
This guide is a work in progress. Its rules can change.
With the style, each file looks as if one person wrote it.
Thus it is easier to read the code, to debug it, to maintain it, to reuse it and to test it.

Most rules of the style are conventions of syntax. Code that obeys the style can still be incorrect.
Obey the style from the first line. That is easier than a change of finished code.
If no rule decides a choice, select the option that makes the code clearer.
Do not write unclear code that this document does not name.

The goal of the style is code that shows each step that it does.
The code has no hidden control flow and no hidden allocation of memory.
Many problems of C++ code come from steps that the code does not show.
Thus this guide removes each feature of C++ that hides a step.

The features that the code can use are in *The features of C++23*.
A program can check some rules of this guide. The check of the style checks these rules.
`[style]` of the config gives the limits of the rules F3, F4, F5, O3 and C1.
Each example of code is a part of a file. It can use names that the rest of the file declares or uses.

## O - The files

### O1 - The contents of the repository

The repository holds only the files that are necessary to build the project, to test it and to understand it.
Do not track a file that a build makes, for example a file `.o`, `.a`, `.so`, `.dll` or `.exe`.
Do not track a temporary file, for example `a.out`, a file `.d`, or a name that ends with `~`.
A build puts its files into `build`, which git ignores.

### O2 - The extension of a file

A source in C++ has the extension `.cpp` or `.hpp`, or `.cppm` for the interface of a module.
`include` holds the files `.hpp`, and the shared headers `.h` of H5.
A source has no other extension. `src` can also hold other inputs of a build.

### O3 - The coherence of a file

A source matches one logical entity, and holds all the functions of that entity.
A file has at most 5 functions. Divide a larger entity into smaller entities, and put each one into its own file.

### O4 - The names of the files and of the folders

The name of a file tells which logical entity the file holds. Thus the name is clear, precise and explicit.
For example, `string.cpp` and `algo.cpp` are bad names. `string_toolbox.cpp` and `pathfinding.cpp` are better names.

Each name of a file or of a folder is in English and in snake case: lower-case letters, digits and underscores only.
This rule applies to each file of the project, also to a document.
A name that a tool needs, which starts with a dot, is an exception, for example `.gitignore`.
You can use an abbreviation if it makes the name much shorter and keeps its meaning.

## G - The global scope

### G1 - The legal notice

Each file of code starts with the legal notice of `.toolchain/header.txt`.
A file of C++ writes the notice between `/*` and `*/`, with each line after `** `:

```text
/*
** Copyright (C) <year> <holder>
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
```

- `<year>` is the year of the first version of the file: four digits, or two years with a `-` between them.
- `<holder>` is a holder of the rights in the file.

If a file holds code of a different holder, add a line of the copyright for that holder below the first line:

```text
/*
** Copyright (C) 2026 <holder>
** Copyright (C) 2005-2020 Rich Felker, et al. (musl, MIT)
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
```

The legal notice gives no path and no description of the file.

### G2 - The separation of the functions

In a source, exactly one empty line follows each function, if the function is not at the end of the file.

### G3 - The indentation of the directives

Indent each directive of the preprocessor by its level. Each level is 4 spaces.
`#if`, `#ifdef` and `#ifndef` open a level, and `#endif` closes it. `#else` and `#elif` are at the level of their `#if`.
Do not put a space between `#` and the name of the directive.

Only a directive takes these levels. A line of code keeps the level of its code, L2.

```cpp
#ifndef WIN32
    #if defined(__i386__) || defined(__x86_64__)
constexpr size_t PAGE_SIZE = 4096;
    #else
        #error "Unknown architecture"
    #endif
#endif
```

### G4 - The global variables

Define each constant that is not trivial as a constant global variable: a typed `constexpr`, not a macro.
If possible, do not use a global variable that is not constant.
A variable is constant only if `const` or `constexpr` marks it. Be careful: the rules of `const` can be surprising.

The first value of each global variable is a constant expression. Thus no code runs before `main` or after `main`.
This also applies to a `static` variable in a function and to a `thread_local` variable.
Mark each of these variables `constexpr`, or `constinit` if its value can change. Then the compiler proves that the first value is constant.

```cpp
constexpr double GOLDEN_RATIO = 1.61803398875;

static uint32_t count_frame(void)
{
    static constinit uint32_t frames = 0;

    frames++;
    return (frames);
}
```

### G5 - static

Mark a global variable or a function with `static` if no other compilation unit uses it.
Be careful: `static` has more than one use.

A named namespace is allowed. Write its qualifier at each use of its names, also inside the namespace, for example `storage::is_alive(r)`.
Thus no call reaches a function through the namespace of an argument.

Do not write `using`, and do not declare a namespace with no name. Use `static` in its place.
A shared header has no namespace, H5. It keeps the prefix of the owner, F2.

```cpp
static bool is_even(uint32_t value)
{
    return (value % 2 == 0);
}

namespace storage {

bool is_alive(const storage_t *storage, uint64_t id);

}
```

A call writes the qualifier:

```cpp
if (!storage::is_alive(&world, id))
    return (false);
```

## F - The functions

### F1 - The coherence of a function

A function does one thing only. It does not mix levels of abstraction.
A function has one responsibility: only one reason can change it.
For example, a call to `malloc()`, a call to `allocate_user()` and a call to `create_user()` are at three different levels of abstraction.

### F2 - The names of the functions

The name of a function tells the task of the function, and contains a verb.
For example, `voyals_nb()` and `dijkstra()` are bad names. `get_voyals_number()` and `get_shortest_path()` are better names.
Each name of a function is in English and in snake case.
You can use an abbreviation if it makes the name much shorter and keeps its meaning.

The verb tells if the call changes something.
A function that answers yes or no starts with `is_` or `has_`. A function that gives a value and changes nothing starts with `get_`.
The name of each other function starts with the verb of its action.
If the name starts with the name of the owner of the function, keep the owner first, for example `storage_is_alive()`.

This code obeys the rule:

```cpp
bool has_moved(const storage_t *r, uint64_t id);
set_t *get_set(storage_t *r, uint32_t slot);
uint64_t hash_type(const type_t *type);
```

This code breaks the rule:

```cpp
bool moved(const storage_t *r, uint64_t id);
set_t *set_of(storage_t *r, uint32_t slot);
uint64_t fingerprint(const type_t *type);
```

These points also hold:

- A function of a type takes the type as its first parameter. A function of a table of V4 takes the context first.
- This parameter is a pointer if the type is a structure. A1 tells when the pointer is `const`.
- The name of a function of a type starts with the name of its owner: the name of the type without `_t`. Or the function is in a namespace with that name, G5.
- No two functions have the same name. Do not overload a function.

```cpp
bool storage_is_alive(const storage_t *storage, uint64_t id);
set_t *storage_get_set(storage_t *storage, uint32_t slot);
float vec3_get_dot(vec3_t a, vec3_t b);
float vec4_get_dot(vec4_t a, vec4_t b);
```

### F3 - The number of columns

A line has at most 80 columns.
A column is not a character. A tab is one character, and it moves the next character to the next multiple of 8 columns.

### F4 - The number of lines

Keep the body of a function as short as possible.
The body of a function has at most 20 lines. A longer function probably does too many tasks.
The body is the lines between the line of the opening bracket and the line of the closing bracket.

This function has a body of 2 lines:

```cpp
int main(void)
{
    printf("hello, world\n");
    return (0);
}
```

### F5 - The arguments

A function that takes no argument takes `void` in its declaration.
C++ lets `f()` declare such a function, but this rule needs `f(void)`.

This code obeys the rule:

```cpp
phys_addr_t alloc_frame(void);
```

This code breaks the rule:

```cpp
phys_addr_t alloc_frame();
```

A function has at most 4 arguments. A variadic function is allowed. Its `...` counts as one argument.
Give a structure to a function through a pointer, and not as a copy.

These points also hold:

- A parameter has no default value. Each call writes each argument.
- A parameter is not a reference, V3.
- A vector or a matrix is a value and not a structure. Give it as a copy, A5.
- A variadic function uses the `...` of C and `<stdarg.h>`. A parameter pack is refused, see *The refused features that hide a cost*.

```cpp
void storage_clear(storage_t *storage);
size_t get_text_length(const char *text);
```

### F6 - The comments

No file has a comment that is not the legal notice.
A function explains itself with its name, its arguments and its code.
If a function needs a comment, make the function shorter or divide it.

## L - The layout in a function

### L1 - The contents of a line

A line holds one statement only. These lines break the rule:

- a line with more than one statement,
- a condition of `if`, `while` or `switch` that holds an assignment,
- a condition and its statement on one line,
- statements that a comma joins,
- a line with more than one assignment.

```cpp
a = b = c = 0;
a++; b++;
if ((ptr = get_item(list)) != nullptr)
if (cond) return (ptr);
a = do_something(), 5;
```

### L2 - The indentation

Each level of indentation is 4 spaces. Do not use a tab for indentation.
A `case` is one level in its `switch`. The statements of a `case` are one level in the `case`.
A closing bracket is at the level of the statement that it closes.

```cpp
switch (kind) {
    case SCHEMA_BOOL:
        return (1);
    default:
        return (0);
}
```

### L3 - The spaces

Put a space after a comma. Put a space after a keyword that has arguments, for example `if`, `while` and `return`.
Do not put a space between the name of a function and its parenthesis. Do not put a space after a unary operator.

Put a space on the two sides of each binary operator and of each ternary operator.
`sizeof` is a unary operator. Thus write `sizeof(x)`.
A line does not end with white space.

This code obeys the rule:

```cpp
return (1);
break;
sum = term1 + 2 * term2;
s = sizeof(file_t);
i++;
```

This code breaks the rule:

```cpp
return(1);
i ++;
```

These points also hold:

- Put a space after `template` and after `requires`.
- Put the clause `requires` on the line after `template`, one level deeper.
- Do not put a space before `<` of template arguments, after `<` or before `>`.
- Write a cast as you write `sizeof`, for example `static_cast<float>(count)`.
- Do not put a space before `::` or after `::`.
- The `:` of a loop over an array has a space on each side.

```cpp
ratio = static_cast<float>(done) / static_cast<float>(total);
pushed = list_push<enemy_t>(&enemies, &enemy);
arch = arch_t::ARM;
```

### L4 - The curly brackets

Put an opening bracket at the end of its line. The opening bracket of a function is alone on its line.
A closing bracket is alone on its line. Only `else` can follow it on the same line.
A block of one line does not need brackets.

This code obeys the rule:

```cpp
while (cond) {
    do_something();
}
if (cond) {
    do_something();
} else {
    do_something_else();
}
if (cond)
    return (ptr);
```

This code breaks the rule:

```cpp
if (cond) {return (ptr);}
if (cond)
{
    do_something();
}
int print_env(void) {
    return (0);
}
```

### L5 - The declarations of the variables

Declare the variables at the start of the function.
You can declare the counter of a `for` loop in the loop.
You can declare a variable and give it a value on the same line.
Declare one variable on a line.

This code obeys the rule:

```cpp
int main(void)
{
    int forty_two = 42;
    int max = 12;

    for (int i = 0; i < max; i++)
        calculate_gcd(forty_two, max);
    return (0);
}
```

This code breaks the rule two times:

```cpp
long calculate_gcd(long a, long b)
{
    long biggest, smallest;

    biggest = MAX(a, b);
    long rest;
    return (biggest);
}
```

### L6 - The empty lines

One empty line separates the declarations of the variables from the rest of the function.
A function has no other empty line.

This code obeys the rule:

```cpp
int sys_open(const char *path)
{
    int fd = thread_reserve_fd();
    filehandler_t *fhandler = nullptr;

    if (fd < 0)
        return (-1);
    thread_set_fd_handler(fd, fhandler);
    return (fd);
}
```

This code breaks the rule:

```cpp
int sys_open(const char *path)
{
    int fd = thread_reserve_fd();

    if (fd < 0)
        return (-1);

    return (fd);
}
```

## V - The variables and the types

### V1 - The names of the identifiers

Each identifier is in English and in snake case.
The names of the macros, of the global constants and of the items of an `enum` are in upper case, for example `UPPER_CASE`.
A constant in a function can also have a name in upper case.
You can use an abbreviation if it makes the name much shorter and keeps its meaning.

This code obeys the rule:

```cpp
#define IS_PAGE_ALIGNED(x) (!((x) & (PAGE_SIZE - 1)))

constexpr double PI = 3.14159;
```

This code breaks the rule:

```cpp
typedef int age;
```

These points also hold:

- The name of each type ends with `_t`: a structure, an `enum class` and a `typedef`.
- A type has one name. Give the name with `_t` to the structure itself, and add no `typedef`.
- An enumeration is an `enum class` with a name and a width.
- Write the name of the `enum class` and `::` before an item, for example `arch_t::ARM`.
- The name of a concept is in snake case, and does not end with `_t`.
- A shared header uses the forms of C for a structure and for an enumeration, H5.

```cpp
enum class arch_t : uint32_t {
    I386 = 0,
    X86_64,
    ARM,
};

struct pixel_t {
    int32_t x;
    int32_t y;
};
```

### V2 - The structures

Put variables into a structure only if they make one coherent entity.
Keep a structure as small as possible.

This code obeys the rule:

```cpp
struct person_t {
    char *name;
    uint32_t age;
    float salary;
};
```

This code breaks the rule:

```cpp
struct trashy_t {
    person_t player;
    uint32_t width;
    uint32_t length;
    uint32_t score;
    int32_t i;
};
```

These points also hold:

- A structure holds fields only. It has no member function, no constructor and no destructor. A5 refuses each `operator`.
- Each structure is trivially copyable and has a trivial destructor. The compiler then copies its bytes, and a copy or the end of a scope calls no function.
- A field has no default value. Give the values when you make the variable, for example with `= {}`.
- Initialize a structure with designators, for example `= {.x = 1, .y = 2}`, or with `= {}`. An array can take its values by position.
- Write `struct`. Do not write `class`, `public`, `private` or `protected`.
- A structure has no base and no `virtual` function. Use an interface, V4.

A class with a constructor and a method becomes a structure and its functions:

```cpp
struct counter_t {
    uint32_t value;
    uint32_t step;
};

counter_t counter_create(uint32_t step)
{
    counter_t counter = {.value = 0, .step = step};

    return (counter);
}

void counter_add(counter_t *counter)
{
    counter->value += counter->step;
}
```

### V3 - The pointers

Attach the pointer symbol `*` to the name of the variable, with no space.
This rule applies to a pointer only. A multiplication is not a pointer.

This code obeys the rule:

```cpp
int32_t *a;
int32_t b = 3 * c;
size_t get_text_length(const char *text);
```

This code breaks the rule:

```cpp
int* a;
```

Do not declare a reference, with `&` or with `&&`. Use a pointer. A1 tells when the pointer is `const`.

### V4 - The interfaces

An interface lets one call reach a different function for each kind of value.
A value of an interface holds a pointer to a table of functions and a pointer to its context.
The table is a structure of pointers to functions. Each kind of value has one `constexpr` table, and its values share it.

The first parameter of each function of the table is the context. The function converts it with `static_cast`, A6.
A pointer of the table can be `nullptr`. Before a call through the table, examine the pointer.

```cpp
struct reader_ops_t {
    size_t (*read)(void *context, uint8_t *data, size_t size);
    void (*close)(void *context);
};

struct reader_t {
    const reader_ops_t *ops;
    void *context;
};

static size_t file_read_bytes(void *context, uint8_t *data, size_t size)
{
    const file_t *file = static_cast<const file_t *>(context);

    return (fread(data, 1, size, file->stream));
}

constexpr reader_ops_t FILE_READER_OPS = {
    .read = file_read_bytes,
    .close = nullptr,
};

size_t reader_read(const reader_t *reader, uint8_t *data, size_t size)
{
    if (reader->ops->read == nullptr)
        return (0);
    return (reader->ops->read(reader->context, data, size));
}
```

A value of the kind file points to the table of that kind:

```cpp
reader_t reader = {.ops = &FILE_READER_OPS, .context = &file};
```

## C - The control structures

Each control structure of C is allowed, if no rule refuses it.
A control structure that only C++ has is refused, except the loop of C4 and `if constexpr` of A4.

### C1 - The conditional branches

A chain of `if` and `else` has at most 3 branches.
In place of a longer chain, use an array of pointers to functions, or a `switch`.

Do not nest conditional branches more than 2 levels deep.
If a function needs more levels, divide it into smaller functions.
`if`, `while`, `for`, `switch` and `do` are levels. An `else if` is not a new level.

This code obeys the rule:

```cpp
if (a)
    do_something();
else if (b)
    do_something_else();
else
    do_something_more();
while (a)
    if (b)
        do_something();
```

This code breaks the rule two times:

```cpp
if (a)
    do_something();
else if (b)
    do_something_else();
else if (c)
    do_something_more();
else
    do_one_last_thing();
while (a)
    for (;;)
        if (b)
            do_something();
```

### C2 - The ternary operator

Use a ternary only if it stays simple and easy to read.
Do not nest or chain ternaries.
Do not use a ternary to control the flow of the program.

This code obeys the rule:

```cpp
parity_t year_parity = (year % 2 == 0) ? parity_t::EVEN : parity_t::ODD;
return (a > 0 ? a : 0);
```

This code breaks the rule:

```cpp
int a = b > 10 ? c < 20 ? 50 : 80 : e == 2 ? 4 : 8;
already_checked ? go_there() : check();
```

### C3 - goto

Do not use `goto`. It quickly makes the code difficult to read.
In rare cases, `goto` makes the code clearer or faster, for example in the handling of errors. Then give the reason in an exception.

### C4 - The loops over an array

A loop `for (item : array)` is allowed only over an array of C. This loop calls no function.
Over a different set of items, use an index.

```cpp
constexpr float WEIGHTS[] = {0.5f, 0.25f, 0.25f};

static float get_total_weight(void)
{
    float total = 0.0f;

    for (float weight : WEIGHTS)
        total += weight;
    return (total);
}
```

## A - The advanced rules

### A1 - The constant pointers

If a function does not change the data of a pointer, mark the pointer `const`.
This applies to a parameter and to a variable of the function.

### A2 - The types

Use the most precise type for the data, for example `size_t`, `ptrdiff_t`, `uint8_t` or `int32_t`.

Do not write `auto`. Write the type of each variable, of each parameter and of each result.

This code obeys the rule:

```cpp
uint32_t counter;
size_t get_obj_size(const void *object);
```

This code breaks the rule:

```cpp
int counter;
auto size = get_obj_size(object);
```

### A3 - The atomics

Use the atomic types of `<stdatomic.h>` by their names, for example `atomic_uint_least32_t` or `atomic_size_t`.
Do not write the keyword `_Atomic`. C++ has no keyword `_Atomic`, and a header of C can also be a header of C++.

Use an atomic value only through a function of `<stdatomic.h>` whose name ends with `_explicit`. Give the memory order in each call.
An operator on an atomic value is refused, for example `count++` or `count = 1`.
If a function has an `_explicit` form, use that form.
`atomic_init`, `atomic_thread_fence`, `atomic_signal_fence` and `atomic_is_lock_free` have no such form, and are allowed.

A thread, a mutex or an event comes from the OS layer of the project, and never from `<threads.h>`.

This code obeys the rule:

```cpp
struct pool_t {
    atomic_uint_least32_t claim;
};

uint32_t pool_claim(pool_t *pool)
{
    return (atomic_fetch_add_explicit(&pool->claim, 1, memory_order_relaxed));
}
```

This code breaks the rule:

```cpp
pool->claim++;
atomic_fetch_add(&pool->claim, 1);
```

### A4 - The templates

A template is allowed only for a generic type, for the functions of that type, and for the concepts of their parameters.

- A parameter of a template is a type, or a value of an integer type or of an `enum class`.
- Each type parameter has a concept, with the clause `requires`. A value parameter is never a pointer or a float.
- A concept is also a template. Its own parameter has no concept.
- Do not write an explicit or a partial specialization.
- Each use writes its template arguments, the types and the values. The compiler does not deduce them.
- A variable template follows the same rules, for example `ZERO<float>`.
- `if constexpr` is allowed only in a template, and only with a condition that reads a parameter of the template.

```cpp
template <typename item_t>
concept list_item = requires (item_t a, item_t b) {
    a = b;
};

template <typename item_t>
    requires list_item<item_t>
struct list_t {
    item_t *items;
    size_t count;
    size_t capacity;
};

template <typename item_t>
    requires list_item<item_t>
[[nodiscard]] bool list_push(list_t<item_t> *list, const item_t *item)
{
    if (list->count == list->capacity)
        return (false);
    list->items[list->count] = *item;
    list->count++;
    return (true);
}
```

A call writes the type of the item:

```cpp
if (!list_push<enemy_t>(&enemies, &enemy))
    return (false);
```

### A5 - The operators and the math types

Do not declare an `operator`, also not a conversion operator. Use the operators that the language gives.

A math type is a vector type or a matrix type of clang.
The operators of a vector type work on each element and call no function.
The operators of a matrix type also work on each element, except the `*` of two matrices.
A product that does not work on each element is a named function, for example `vec3_get_dot` or `vec3_get_cross`.
The `*` of two matrices is such a product. Only its named function uses it, for example `mat4_get_product`.

A vector and a matrix are values. Give them to a function as a copy.
The matrix types need `-fenable-matrix`, see *The flags*.

```cpp
typedef float [[clang::ext_vector_type(3)]] vec3_t;
typedef float [[clang::matrix_type(4, 4)]] mat4_t;

float vec3_get_dot(vec3_t a, vec3_t b)
{
    vec3_t product = a * b;

    return (product.x + product.y + product.z);
}

mat4_t mat4_get_product(mat4_t a, mat4_t b)
{
    return (a * b);
}
```

An operator on vectors works on each element:

```cpp
position = velocity * 2.0f + start;
```

### A6 - The conversions

Write each conversion that can lose a value or change a sign. Use `static_cast`.
Do not use a cast of C, a functional cast such as `float(x)`, or `const_cast`. They do not show which conversion they do.
No conversion calls a function, because a structure has no constructor, V2, and no conversion operator, A5.

Read only the member of a union that was written last.
Use `reinterpret_cast` only to a pointer to `unsigned char`, `uint8_t` or `char`, to read the bytes of a value.
An intrinsic that loads or stores a vector of the processor can also take a pointer of its vector type, A10.
To read the bytes of a value as a different type, copy them with `memcpy`.

```cpp
uint8_t low = static_cast<uint8_t>(value & 0xffu);
const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&header);
memcpy(&bits, &ratio, sizeof(bits));
```

### A7 - The allocations

A function that allocates memory takes an allocator as a parameter. Thus each allocation shows in the call.
Do not use `new` or `delete`. An allocator is an interface, V4.

These functions of the C library allocate or release memory: `malloc`, `calloc`, `realloc`, `aligned_alloc` and `free`.
Where the C library has them, `free_sized`, `free_aligned_sized`, `strdup` and `strndup` do too.
Only the functions of an allocator call them.
The allocator gives `void *`. Convert it to the type of the data with `static_cast`.

A value that holds memory has a function that releases it. The caller calls this function, because no destructor does.

```cpp
struct allocator_ops_t {
    void *(*allocate)(void *context, size_t size, size_t alignment);
    void (*release)(void *context, void *memory, size_t size);
};

struct allocator_t {
    const allocator_ops_t *ops;
    void *context;
};

[[nodiscard]] bool buf_create(buf_t *buf, const allocator_t *alloc, size_t size)
{
    void *memory = nullptr;

    if (alloc->ops->allocate == nullptr)
        return (false);
    memory = alloc->ops->allocate(alloc->context, size, alignof(uint8_t));
    if (memory == nullptr)
        return (false);
    buf->data = static_cast<uint8_t *>(memory);
    buf->size = size;
    return (true);
}

void buf_destroy(buf_t *buf, const allocator_t *alloc);
```

A function that makes 2 values releases the first value when the second one fails:

```cpp
[[nodiscard]] bool pair_create(pair_t *pair, const allocator_t *alloc, size_t size)
{
    if (!buf_create(&pair->first, alloc, size))
        return (false);
    if (!buf_create(&pair->second, alloc, size)) {
        buf_destroy(&pair->first, alloc);
        return (false);
    }
    return (true);
}
```

### A8 - The errors

A function that can fail returns its error as a value: a `bool` or an `enum class` of errors.
`[[nodiscard]]` marks each function that returns an error. The caller examines the result.
Do not cast the result to `void`, also not with `static_cast<void>`.

```cpp
enum class read_error_t : uint32_t {
    NONE = 0,
    END,
    DEVICE,
};

[[nodiscard]] read_error_t file_read(file_t *file, uint8_t *data, size_t size);
```

The function gives each failure as an item of the enumeration:

```cpp
read_error_t file_read(file_t *file, uint8_t *data, size_t size)
{
    size_t count = fread(data, 1, size, file->stream);

    if (count == size)
        return (read_error_t::NONE);
    if (feof(file->stream) != 0)
        return (read_error_t::END);
    return (read_error_t::DEVICE);
}
```

### A9 - The extensions of the compiler

Use an extension of the compiler only if this rule names it.
Each other extension is refused, also an extension that this rule does not name.

| Extension | Rule |
| --- | --- |
| `__builtin_align_up` and `__builtin_align_down` | Allowed, to align a pointer or a size. |
| `__builtin_convertvector` | Allowed, to convert each element of a vector. |
| `__builtin_add_overflow`, `__builtin_sub_overflow` and `__builtin_mul_overflow` | Allowed, to compute and to detect an overflow. They replace `<stdckdint.h>`, which some targets lack. |
| `__builtin_popcountg`, `__builtin_clzg` and `__builtin_ctzg` | Allowed, to count bits. They replace `<stdbit.h>`, which macOS and Windows lack. |
| `[[gnu::cold]]`, `[[gnu::hot]]`, `[[gnu::noinline]]` and `[[gnu::always_inline]]` | Allowed. They only guide the compiler. |
| `[[gnu::format(...)]]` and `[[gnu::nonnull]]` | Allowed. They add a check. |
| `[[gnu::visibility("hidden")]]` and `[[gnu::visibility("default")]]` | Allowed. |
| `[[clang::ext_vector_type(...)]]` and `[[clang::matrix_type(...)]]` | Allowed, for the vector and matrix types. |
| `__builtin_assume` and `__builtin_unreachable` | Refused. A false condition gives undefined behavior. Examine the condition with `if` or with `assert`. |
| `[[gnu::constructor]]` and `[[gnu::destructor]]` | Refused. Code runs before or after `main`, G4. |
| `[[gnu::packed]]` | Refused. It changes the layout of a structure. |
| `[[gnu::weak]]` | Refused. The linker selects the function of a call. |

This code obeys the rule:

```cpp
[[gnu::cold]] void report_failure(const char *reason);
```

This code breaks the rule:

```cpp
[[gnu::constructor]] void setup(void);
```

### A10 - The instructions of a processor

The intrinsics of a family of processors are allowed, for example `<immintrin.h>` or `<arm_neon.h>`.
Put each use behind `#if` on a macro of the processor, for example `__x86_64__` or `__aarch64__`.
Each such `#if` has an `#else` with the portable code, so that each target builds.
Do not write `asm`. The compiler does not examine its instructions.

This code obeys the rule:

```cpp
#if defined(__x86_64__)
    #include <emmintrin.h>
#endif

void fill_bytes(uint8_t *to, uint8_t byte)
{
#if defined(__x86_64__)
    _mm_storeu_si128(reinterpret_cast<__m128i *>(to), _mm_set1_epi8(static_cast<char>(byte)));
#else
    memset(to, byte, 16);
#endif
}
```

This code breaks the rule:

```cpp
asm("pause");
```

## H - The headers

### H1 - The contents of a header

A header holds only these items:

- the prototypes of functions,
- the declarations of types,
- the declarations of global variables,
- macros,
- functions `static inline`.

These items are only in headers.
A header includes a different header only if it is necessary.
The attributes of a function are in brackets, for example `[[nodiscard]]` and `[[maybe_unused]]`.

A file `.hpp` can also hold these items:

- the templates of the generic types and of their functions, A4,
- the concepts of the parameters of these templates, A4,
- functions `static constexpr` and `static consteval`.

```cpp
static constexpr uint32_t get_mask(uint32_t bits)
{
    return ((1u << bits) - 1u);
}
```

### H2 - The include guard

Protect each header from a second inclusion.
The first two directives are `#ifndef` and `#define` of the same name, or one of them is `#pragma once`.

### H3 - The macros

Do not use a macro for a constant. Use a typed `constexpr`.
A macro holds one statement only.
A macro with no arguments can give the text between the braces of a type: its fields or its values.
Thus a type and the text that describes it are the same text.

This code obeys the rule:

```cpp
constexpr double PI = 3.14159265358979323846;
#define DELTA(a, b, c) ((b) * (b) - 4 * (a) * (c))
```

This code breaks the rule:

```cpp
#define PI 3.14159265358979323846
#define LOOP_COUNTER 0
#define PRINT_NEXT(num) {num++; printf("%d", num);}
```

### H4 - The macros of a text

Write a macro of a text like a type.
The line of `#define` ends with the opening brace.
Each field or value is on its own line, 4 spaces deeper than `#define`.
The closing brace is alone on the last line, at the depth of `#define`.

This code obeys the rule:

```cpp
#define POSITION_FIELDS { \
    float x; \
    float y; \
}
```

This code breaks the rule:

```cpp
#define POSITION_FIELDS { float x; float y; }
```

### H5 - The shared headers

A shared header is a file `.h` of `include` that C and C++ both read. A header for C++ only is a file `.hpp`.
A shared header holds only what C23 and C++23 both accept:

- the structures in the form of C, `typedef struct { ... } name_t;`,
- the enumerations of C23 with a width, for example `enum kind : uint32_t {`,
- the constants `constexpr`,
- the declarations of functions.

In a shared header, these forms replace the forms of V1. Out of a shared header, they are refused.
Write `enum kind` as the type of an enumeration, because C needs the keyword.
Put `extern "C"` around the declarations of the functions, behind `#ifdef __cplusplus`. The block of `extern "C"` adds no level, L2.
A shared header is also a header of C, so the style of the C code of the project also applies to it.

```c
#ifndef READER_H
    #define READER_H

    #include <stddef.h>
    #include <stdint.h>

constexpr uint32_t READER_VERSION = 1;

enum reader_kind : uint32_t {
    READER_FILE = 1,
    READER_MEMORY = 2,
};

typedef struct {
    enum reader_kind kind;
    size_t size;
} reader_info_t;

    #ifdef __cplusplus
extern "C" {
    #endif

[[nodiscard]] bool reader_get_info(const char *path, reader_info_t *info);

    #ifdef __cplusplus
}
    #endif
#endif
```

## The spellings

The code of C++ uses the spellings of the right column. Each spelling of the left column is a fault.

| Fault | Spelling |
| --- | --- |
| `NULL` or `0` as a pointer | `nullptr` |
| `#include <stdbool.h>` | Nothing. `bool`, `true` and `false` are keywords. |
| `_Bool`, `_Alignof`, `_Alignas` or `_Static_assert` | `bool`, `alignof`, `alignas` or `static_assert` |
| `_Thread_local` or `THREAD_LOCAL` | `thread_local` |
| `_Noreturn` | `[[noreturn]]` |
| `__attribute__` or `__declspec` | An attribute in brackets, for example `[[gnu::cold]]` |
| `(void)name;` | `[[maybe_unused]]` on the declaration of the parameter |
| `= {0};` | `= {};` |
| `#define NAME 4`, a macro for a number | A typed `constexpr`, for example `constexpr int32_t NAME = 4;` |
| `#include <cstdio>`, a header of the C++ library | `#include <stdio.h>` |
| `enum kind : uint32_t {`, an `enum` with no `class` | `enum class kind_t : uint32_t {` |
| `enum class kind_t {`, an `enum class` with no width | The `enum class` with its width |
| `typedef struct point {`, a second name for a structure | `struct point_t {` |
| `struct point_t p;`, a keyword before the name of a type | `point_t p;` |

## The features of C++23

This guide is an allow list. The code uses only the features that *The allowed features* allows.
Each other feature is refused, also a feature that this guide does not name.
A feature of a standard after C++23 is refused until this guide allows it by name.

An exception can excuse a rule of the groups O to H. It cannot allow a feature that this section refuses. Only a change of this guide can allow it.

The refused features are in groups, by what they hide. Each row gives the reason, and what to write in its place.

### The allowed features

| Feature | What it hides | Rule |
| --- | --- | --- |
| The features of C23 that C++23 also has | Only what they hide in C | Allowed, if no rule of this guide refuses it. |
| `static_assert`, `alignas`, `alignof` and the digit separator `'` | Nothing | Allowed. |
| `[[maybe_unused]]`, `[[fallthrough]]` and `[[noreturn]]` | Nothing | Allowed. |
| `[[nodiscard]]` | Nothing. It adds a check. | Allowed, on each function that returns an error, A8. |
| `[[likely]]` and `[[unlikely]]` | Nothing. They only guide the compiler. | Allowed. |
| The suffixes `z` and `uz` of an integer | Nothing. They give the signed type of `size_t`, and `size_t`. | Allowed. |
| The headers of the C library | Only what they hide in C | Allowed only in the list of *The allowed headers of the C library*. |
| `constexpr` variables and `constinit` | Nothing. They add a check. | Allowed, G4. |
| `constexpr` and `consteval` functions | A `constexpr` function can run at run time, as a call that the code shows. | Allowed, H1. |
| Designated initializers, also nested, `.in.a = 1` | Nothing | Allowed, with the fields in their order. Nested designators need the flag `-Wno-c99-designator` of *The flags*. |
| `#embed` | Nothing. It gives the bytes of a file where it is written. | Allowed. It needs the flag `-Wno-c23-extensions` of *The flags*. The check refuses each other extension of C23 that this flag lets through. |
| `_BitInt(N)` | Nothing | Allowed. It needs the flag `-Wno-bit-int-extension` of *The flags*. |
| `enum class` | Nothing. It removes a conversion. | Allowed, with a width, V1. |
| The operator `::` | Nothing | Allowed after the name of an `enum class`, V1, and after the name of a namespace, G5. |
| A named namespace | Nothing, when each use writes its qualifier | Allowed, G5. |
| The name of a structure as a type, with no `struct` | Nothing | Allowed, V1. |
| `nullptr` | Nothing | Allowed. |
| `extern "C"` | Nothing. It shows the linkage of C. | Allowed only in a shared header, H5. |
| The loop over an array of C | Nothing | Allowed, C4. |
| `goto` | A jump that the code shows | Allowed only with an exception of C3. |
| Modules: `export module`, `import` and `export` | Nothing at run time. The build orders the sources: an import needs the compiled interface of its module first. | Allowed. The interface of a module is a file `.cppm`. A shared header of H5 stays the seam with C, because C has no modules. |
| The extensions of the compiler | Only what each one hides | Allowed only in the list of A9. |
| The intrinsics of a family of processors | Nothing. Each one names its instruction. | Allowed behind `#if`, with a portable `#else`, A10. |
| `if constexpr` | A branch that the compiler removes | Allowed only in a template, A4. |
| Templates and concepts | Code for each type | Allowed with the conditions of A4. |
| Value parameters of a template and variable templates | Nothing more than a template: the call writes them | Allowed with the conditions of A4. |
| The vector and matrix types of clang | Nothing. Their operators are built in. | Allowed, A5. |
| The swizzles of a vector, for example `v.xy` | Nothing. They name the elements. | Allowed, A5. |
| Raw string literals, `R"(...)"` | Nothing | Allowed. |
| Named and delimited escapes, `\N{...}` and `\x{...}` | Nothing | Allowed. |
| The macros that test a feature, `__cpp_*` and `__has_cpp_attribute` | Nothing | Allowed. |
| `static_cast` | Nothing. It shows the conversion. | Allowed, A6. |
| `reinterpret_cast` | A break of the rules of aliasing, if the new type is not a type of bytes | Allowed only to a pointer to `unsigned char`, `uint8_t` or `char`, A6. |
| The implicit copy, move, constructor and destructor | Calls that the code does not show | Allowed only when trivial, V2. |

### The allowed headers of the C library

The code includes only these headers of the C library, and the headers of the intrinsics of A10. They build on each platform, also with `-nostdlib++`.

```text
<assert.h>    <ctype.h>     <errno.h>     <fenv.h>      <float.h>
<inttypes.h>  <limits.h>    <locale.h>    <math.h>      <signal.h>
<stdarg.h>    <stdatomic.h> <stddef.h>    <stdint.h>    <stdio.h>
<stdlib.h>    <string.h>    <time.h>      <wchar.h>     <wctype.h>
```

### The refused features that hide a type

| Feature | What it hides | Rule |
| --- | --- | --- |
| `auto` | The type of the variable | Refused. Write the type, A2. |
| `decltype` | The type of an expression | Refused. Write the type, A2. |
| Trailing return types, `auto f(void) -> int32_t` | The type of the result, at the start of the declaration | Refused. Write the type first, A2. |
| Class template argument deduction | The arguments of a template | Refused. Write the arguments, A4. |
| Aliases with `using` | A second form of `typedef`, also for a template | Refused. Write `typedef`, V1. |
| `using enum` | The name of the enumeration of an item | Refused. Write `kind_t::ITEM`, V1. |
| A cast from one vector type to another | A copy of the bits, not a conversion of each element | Refused. Write `__builtin_convertvector`, A5. |
| `char8_t` and the prefix `u8` | A second type of text. The C library, the OS and the shared headers take `char`, so each use needs a cast, and the type differs between C and C++. | Refused. A text of the project is UTF-8 in `char`. Write `uint8_t` for bytes. |
| The casts of C, the functional casts and `const_cast` | Which conversion the cast does | Refused. Write `static_cast` or `reinterpret_cast`, A6. |
| Lossy implicit conversions | A loss of a value, or a change of the sign | Refused, A6. |
| References | A call `f(x)` that can change `x` | Refused, V3. |
| `mutable` | A change of a value that is `const` | Refused. Do not mark the value `const`, A1. |
| Structured bindings | Aliases, and calls to `get` | Refused. Declare one variable on a line, L5. |
| RTTI: `typeid` and `dynamic_cast` | A test of the type at run time, and tables of types | Refused. Keep the kind of a value in a field. |

### The refused features that hide a call

| Feature | What it hides | Rule |
| --- | --- | --- |
| Member functions | The pointer `this` | Refused, V2. |
| Constructors, also converting constructors and `explicit` | A call in `x_t x = 5;` | Refused, V2. |
| `= default` and `= delete` | They make or remove a special member function or an overload. V2 keeps each structure trivial, and F2 refuses overloads. | Refused. Write nothing, V2 and F2. |
| User destructors | A call at the end of a scope | Refused, V2. |
| Default values of fields | A constructor that the code does not show | Refused, V2. |
| A declaration of an `operator`, also a conversion operator | A call in `a + b` or in `int32_t i = x;` | Refused, A5. |
| User-defined literals, `operator ""` | A call in `12_km` | Refused, A5. |
| The three-way comparison `<=>` | A call of an operator, and the header `<compare>` of the C++ library | Refused. Compare with `<` and `==`, or call a named function. |
| Default arguments | A value that the call does not show | Refused, F5. |
| Default arguments of a template | An argument of a template that the use does not show | Refused. Each use writes its arguments, A4. |
| The short forms of a constraint, `template <list_item item_t>` and `list_item auto x` | A second form of `requires`, and `auto` | Refused. Write the clause `requires`, A4. |
| An explicit instantiation and `extern template` | A copy of a template that no use makes | Refused. A use that writes its arguments makes the copy, A4. |
| Pointers to members, `&point_t::x`, `.*` and `->*` | An offset or a call through a hidden table | Refused. Use `offsetof` of `<stddef.h>`, or an interface, V4. |
| A list initialization with no `=`, `point_t p{1, 2};` | It looks like a call, and it gives the values by position | Refused. Write `point_t p = {.x = 1, .y = 2};`, V2. |
| Function overloading | The types of the arguments select the function | Refused, F2. |
| Parenthesized aggregate initialization, `point_t p(1, 2);` | It looks like a call to a constructor | Refused. Write `point_t p = {.x = 1, .y = 2};`. |
| The directive `using`, a declaration `using`, and a namespace with no name | Names that come into a scope with no qualifier | Refused, G5. |
| A call with no qualifier to a name of a namespace | The lookup that the arguments select | Refused. Write the qualifier, G5. |
| Lambdas | With captures, copies of variables. With no capture, a type with no name, and an implicit conversion to a pointer to a function. | Refused. Write a named `static` function, G5 and F2. |
| `virtual`, inheritance, `final` and `override` | A call through a hidden table, a hidden pointer in each value, and a change of the layout and of the pointer | Refused. Use an interface, V4, or put the other structure in a field. |
| `std::function` | A hidden type, an allocation and a call through a pointer | Refused. Use an interface, V4. |
| `std::expected`, `std::optional` and `std::span` | Calls of operators, and a stop on a bad access | Refused. Return the error, A8, and give the result through a pointer. |
| `<complex.h>` and `<tgmath.h>` | Overloads: the functions of `<tgmath.h>` are `overloadable`, and the type `_Complex` is not C++ | Refused. Use `<math.h>` and the math types, A5. |
| Static initialization that runs code | Code before `main`, in an order that is not known | Refused, G4. |
| `atexit` and `at_quick_exit` | Code after `main` | Refused. Before `main` returns, call the functions that release the values, A7. |

### The refused features that hide a jump

| Feature | What it hides | Rule |
| --- | --- | --- |
| `throw`, `try` and `catch` | A jump out of a call that the call does not show | Refused. Return the error as a value, A8. |
| `noexcept` | A stop of the program if a `throw` leaves the function | Refused. Write nothing, because no code throws. |
| `<setjmp.h>` | A jump out of a function that the call does not show | Refused. Return the error as a value, A8. |
| `[[reproducible]]` and `[[unsequenced]]` | Promises of C23 about the effects of a function. A false promise gives undefined behavior. | Refused. Write nothing. |
| `[[carries_dependency]]` | An order of memory of `memory_order_consume`, which compilers treat as `acquire` | Refused. Give the order with the `_explicit` functions, A3. |
| `[[assume]]` | Undefined behavior when its condition is false | Refused, as `__builtin_assume`, A9. |
| `if consteval` | A selection of the code by the context of the call | Refused. Write a `consteval` function and a different function for run time. |
| `if`, `switch` and the loop over an array, with an init statement | A declaration in the head of a control structure | Refused. Declare the variable at the start of the function, L5. |
| Coroutines | Hidden points of suspension, and a frame on the heap | Refused. Keep the state in a structure. |
| Declarations `asm` | Instructions that the compiler does not examine | Refused. Use an intrinsic, A10. |

### The refused features that hide a layout

| Feature | What it hides | Rule |
| --- | --- | --- |
| `class`, `public`, `private`, `protected` and `friend` | Rules of access. A structure with mixed access does not have the standard layout. | Refused. Write `struct`, V2. |
| `[[no_unique_address]]` | A field with no bytes, or a field that shares its bytes | Refused. Remove an empty field. |
| Static data members and nested types | A global variable or a type inside a structure, which holds fields only | Refused. Declare them at the file scope, V2 and G4. |
| Inline variables | One variable from a copy in each unit, which the linker merges | Refused. Declare the variable in a header, and define it in one source. |

### The refused features that hide a cost

| Feature | What it hides | Rule |
| --- | --- | --- |
| Parameter packs, fold expressions and variadic templates | Code that the compiler writes for each argument, which the source does not show | Refused. Use the `...` of C with `<stdarg.h>`, F5, or an array and its count. |
| Template template parameters | A template that is passed to a template: a second level of templates | Refused. Pass the types, A4. |
| `new` and `delete` | A call to the heap and to a constructor | Refused. Give an allocator, A7. |
| The C++ standard library: containers, `string`, iostreams and ranges | Allocations, global state, and a stop of the program on a failure | Refused. Use a generic type of the project, A4, or the C library. |

### The refused features of C23 that C++23 does not have

C++23 does not have these features of C23. Clang accepts some of them only as an extension.

| Feature | What it hides | Rule |
| --- | --- | --- |
| Variable length arrays | An allocation on the stack with a size that is not known | Refused. Use a constant size, or an allocator, A7. |
| Flexible array members | A size of the structure that is not its size | Refused. Give the items through a pointer in a field. |
| `typeof` and `typeof_unqual` | The type of an expression, as `decltype` | Refused. Write the type, A2. |
| `_Decimal32`, `_Decimal64` and `_Decimal128` | Clang does not build them in C++ | Refused. Use an integer that counts the smallest unit. |
| `restrict` and `__restrict` | A promise that 2 pointers do not overlap. A false promise gives undefined behavior, as `[[assume]]`. | Refused. Write nothing. |
| `_Generic` | A selection of an expression by the type | Refused. Give each function its own name, F2. |
| Compound literals, `(point_t){1, 2}` | A value with a different lifetime in C++ | Refused. Declare a variable with its values. |
| Array and mixed designators | An index that C++ does not accept, also as an extension | Refused. Name each field, in their order. |
| The keyword `_Atomic` | Not in C++23 | Refused. Write a type of `<stdatomic.h>`, A3. |
| `<threads.h>` | Not in C++23, and not on each platform | Refused. Use the OS layer of the project, A3. |
| `<stdckdint.h>`, `<stdbit.h>` and `<uchar.h>` | Absent on some of the 6 targets: `<stdbit.h>` on macOS and Windows, `<uchar.h>` on macOS. `<stdckdint.h>` is not in C++23. | Refused. Use the builtins of A9. `char16_t` and `char32_t` go with `<uchar.h>`. |

## The flags

Compile each file of C++ with these flags. They make the compiler refuse a part of the faults of this guide.
The column Test gives the version of zig that tested the effect of the flag on code that breaks it.

| Flag | Effect | Test |
| --- | --- | --- |
| `-std=c++23` | Selects the language C++23. | Tested with zig 0.17.1. |
| `-fno-exceptions` | Makes `try` and `throw` compile errors. | Tested with zig 0.17.1. |
| `-fno-rtti` | Makes `typeid` and `dynamic_cast` compile errors. | Tested with zig 0.17.1. |
| `-nostdlib++` | Removes the headers of the C++ library. The headers of the C library still build. | Tested with zig 0.17.1. |
| `-Wall` and `-Wextra` | Turn on the usual warnings. | Tested with zig 0.17.1. |
| `-Werror` | Makes each warning an error. | Tested with zig 0.17.1. |
| `-pedantic-errors` | Makes an error of each extension that the standard refuses. The types of A5 still build. | Tested with zig 0.17.1. |
| `-Wconversion` | Warns about an implicit conversion that can lose a value. | Tested with zig 0.17.1. |
| `-Wsign-conversion` | Warns about an implicit conversion that changes the sign. `-Wconversion` also turns it on; the flag states it. | Tested with zig 0.17.1. |
| `-Wold-style-cast` | Warns about a cast of C. It does not find a functional cast such as `float(x)`. | Tested with zig 0.17.1. |
| `-Wimplicit-fallthrough` | Warns about a `case` that continues into the next `case` with no `[[fallthrough]]`. | Tested with zig 0.17.1. |
| `-Wvla` | Warns about an array with a size that is not constant. C++ already warns about it; the flag states it. | Tested with zig 0.17.1. |
| `-Wreorder-init-list` | Warns about designated initializers that are not in the order of the fields. C++ already warns about it; the flag states it. | Tested with zig 0.17.1. |
| `-Wglobal-constructors` and `-Wexit-time-destructors` | Warn about code that runs before `main` or after `main`. | Tested with zig 0.17.1. |
| `-Wzero-as-null-pointer-constant` | Warns about `0` as a pointer. | Tested with zig 0.17.1. |
| `-fenable-matrix` | Enables the matrix types of A5. | Tested with zig 0.17.1. |
| `-Wno-c23-extensions` | Lets `#embed` build in C++. The check refuses each other extension of C23. | Tested with zig 0.17.1. |
| `-Wno-bit-int-extension` | Lets `_BitInt(N)` build in C++. | Tested with zig 0.17.1. |
| `-Wno-c99-designator` | Lets nested designators build in C++. `-Wextra` still demands each field. | Tested with zig 0.17.1. |

## Exceptions

A file can break a rule only if an exception names the rule and the file, and gives a full and correct reason.
A good reason looks at the long term: clean code, easy reading, flexibility, or a speed that you measured.
Write the reason in the exception, and not in a comment of the code.
Write each exception as a table `[[style.exception]]` of the config:

```toml
[[style.exception]]
rule = "F4"
path = "src/parse.cpp:parse"
why = "The table of the grammar is one long function."
```

`rule` is the rule, and `why` is the reason.
`path` is the path of the file. It can contain `*`, and it can end with `:` and the name of a function.
The check refuses an exception that excuses no fault. Thus no exception stays after its reason or its file goes.

## Attribution

This style comes from the style of the C code of the project: `docs/contributing/coding_style.md`.
That style comes from the Epitech C Coding Style, version 3.1.
The Epitech style comes from the Linux Kernel Coding Style and from the book Clean Code of Robert C. Martin.

The rules of C++ come from these sources:

- The Zig overview, for the goal and for the allocators: https://ziglang.org/learn/overview/
- The Doom 3 Code Style Conventions, for the functions with no overloading: https://fabiensanglard.net/fd_proxy/doom3/CodeStyleConventions.pdf
- The JSF AV C++ Coding Standards, for the conversions, `throw` and the constraints of templates: https://www.stroustrup.com/JSF-AV-rules.pdf
- Array programming in Odin, for the math types with built-in operators: https://github.com/odin-lang/Odin/discussions/2731
- Orthodox C++, for a subset of C++ near to C: https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b
- The C++ features of Chromium, for the modules, the coroutines and `std::function`: https://chromium.googlesource.com/chromium/src/+/main/styleguide/c++/c++-features.md
- The language pages of cppreference, for the features of each standard of C++: https://en.cppreference.com/w/cpp/23
