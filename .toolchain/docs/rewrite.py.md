# .toolchain/tools/rewrite.py

`rewrite.py` makes a changed copy of the code, for `test --branches`, for `test --leaks` and for the cases of `test` that state `fail`.
It holds the parts that these runs share.
`.toolchain/docs/branches.py.md`, `.toolchain/docs/leaks.py.md` and `.toolchain/docs/fail.py.md` tell what each run writes into the copy.

## The steps

1. `sources` finds each artifact that the build makes for this target.
2. It takes each file `.c` of `src` that such an artifact compiles. It uses the flags of the first such artifact and of its kind.
   It also keeps the parts of all the artifacts that compile the file. A file of a project with no parts has no part.
3. `each` runs the command `[rewrite] dump` of `toolchain.toml` for each file, 8 files at the same time. It gives each syntax tree to the run, and gives the parts of the file with its result.
4. `copied` copies `src` and `include` into `<build>/<run>/tree`. The run then writes its changes into the copies.

`test.py` then builds the copies, and runs the cases: `.toolchain/docs/test.py.md`.

## The rename

A run can give a function that a file uses a new name, after the last `#include` of the file. For `free` and the count of `leaks.py`, the block is:

```c
[[maybe_unused]] static typeof(*free) c23b_l_free;
#pragma push_macro("free")
#undef free
#define free c23b_l_free
```

Each later use of the name calls the function of the new name, also a use of the name as a pointer.
No header sees the new name, because the headers come before the block.
If the last `#include` is in an `#if` that holds only lines of the preprocessor, the block comes after the `#endif` of that `#if`.
A `#line` after the block keeps the numbers of the lines.
At the end of the file, `#pragma pop_macro` gives each name its first meaning again. Then the run writes the function of each new name.

`found` finds the functions that a file uses, and their declarations. A function that the file defines does not count, unless it is in `callbacks`.
A use is a reference to a function. A variable with the name of a function, for example a local `free`, is not a use.

## The pointer variables

A library that loads its calls at run time declares each call as a global variable of a pointer to a function. volk for Vulkan is an example.
`found` also takes such a variable as a function, when `names` lists it and its type is a pointer to a function.
`pointed` reads the result and the parameters from the type `R (*)(P1, P2)`.
`split_top` cuts the parameters at the commas that no parentheses hold.

`typeof(*name)` is the type of the function for a function and for a pointer to it. So the rename and the result of `signed` are the same for both.
A file that assigns a value to a listed variable does not build, because the new name is a function. A call site only calls the variable.

`signed` gives the start of the function of a new name. It writes the type of each parameter as the declaration spells it, with no nullability mark such as `_Nullable`.
It also removes the calling conventions `cdecl`, `stdcall`, `fastcall` and `thiscall`, which change nothing on x86_64 and on aarch64.
Thus a `typedef` keeps its name, also a `typedef` whose type holds a calling convention, such as `LPTHREAD_START_ROUTINE`.
It stops on a function with `...`: `<name> takes a list of arguments of no fixed length`.

The result is the type of a call through a pointer to the function type, with an empty value of each parameter type.
For example, the result of `malloc` is `typeof(((typeof(*malloc) *)nullptr)((size_t){}))`. Thus a builtin that gives `size_t`, such as `fread`, builds too.

A call through a pointer reads no attribute of the declaration. Thus no attribute checks the empty values, for example the alignment of `aligned_alloc`.
The function of a new name calls its function as `(name)(a0, a1)`. Thus a macro with arguments of the same name does not apply, for example the C23 `memchr` of glibc.

## The writer

`writer` gives the text that ends each file `.c` of the count of `leaks.py`.
The probes of `branches.py` need no writer: `.toolchain/docs/branches.py.md`, *The hits*.

A `[[gnu::destructor]]` writes one line for the file, at the exit of the program and at the close of a library.
It writes to the file that a variable of the environment names. If the variable is not set, it writes nothing.
The line starts with the path of the file. The count gives the rest of the line.

The writer builds each line in a buffer and writes it with one append, so parallel programs do not interleave.
On Windows, an append is a seek and a write, so 2 programs that end at the same time can write over each other.
Thus on Windows the writer opens the file with `_SH_DENYRW`, and opens it again while another program holds it.
It uses the low-level file calls of the system, with no buffer of `stdio`.

On macOS, a `[[gnu::constructor]]` of each file opens its own library again, with `RTLD_NOLOAD` and `RTLD_NODELETE`.
Thus a closed library stays mapped, and it does not matter which code opens and closes it, for example a released library.
Then its destructor runs at the exit of the program, with the code of the library still in place.
The constructor finds its library with `dladdr`. It declares `dladdr` and `dlopen` with names of its own, so it includes no header.

## The syntax tree

`dump` runs `zig cc` with `-fsyntax-only`, and with the other flags of the artifact.
zig then writes an error ending with `FileNotFound`, because no object is made. `dumped` ignores only such an error.
If clang writes a different error, `dumped` stops: `the tree of <path> cannot be read:`, and the errors.
If the output is not JSON, it stops: `the tree of <path> is not JSON`.

The dump sets `_FORTIFY_SOURCE` to 0. Then a call that the system would fortify, such as `snprintf`, does not read as a macro.

The run builds the files of `inputs` after the dump, so they do not exist yet.
zig gives an `#embed` of a file that does not exist no bytes and no error, and an empty array is an error of `-pedantic-errors`.
Thus `stood_in` writes a file of 1 zero byte for each file of `inputs`, into `<work>.inputs`.
The dump has `--embed-dir=<work>.inputs` in `{inputs}`. The syntax tree needs no byte of an input.
The build of the copies embeds the files that the run builds.

## Limits

- A case that crashes, or that stops before its end, writes no line.
- A type that a declaration spells with a different attribute, for example `vectorcall` or `noreturn`, stops the build of the copy. So does a type name that the end of the file does not know, for example a `typedef` in a block.
- Every file of the count writes with a `[[gnu::destructor]]`. A destructor of a library that `dlclose` unmaps on macOS would run at the exit of the program with its code gone. So on macOS each file keeps its own library mapped, and the destructor runs in place.

## Functions

| Function | Result |
| --- | --- |
| `tree(values, run)` | Gives the folder of the copies of a run, `<build>/<run>/tree`. |
| `marked(node, state)` | Gives each location of a syntax tree the file that it is in. The dump writes a file only when it changes. |
| `is_macro(loc)` | Tells if a location comes from a macro. |
| `owned(loc, root)` | Gives the path of a location in `src` or in `include`, or `None`. |
| `dumped(command, root)` | Runs the command `dump`, and gives the syntax tree, with the file of each location. |
| `stood_in(artifact, mine)` | Writes a file of 1 zero byte for each file of `inputs`, and gives the flag `--embed-dir` of their folder, or no flag. |
| `sources(config, values)` | Gives each file `.c` of `src` that an artifact of this target compiles, with its command `dump` and the set of its parts. |
| `each(config, values, use)` | Gives the result of `use(root, path, tree)` and the sorted parts of the file, for each file of `sources`, 8 files at the same time. |
| `copied(root, where)` | Empties `where`, and copies `src` and `include` into it. |
| `plain(kind)` | Gives a type of the syntax tree with each `typedef` replaced, if the dump gives it. The checks of `leaks.py` read it, for example to find a pointer. |
| `spelled(text)` | Gives a type as the declaration spells it, with no nullability mark and no calling convention that *The rename* names. |
| `declaration(node, typedefs)` | Gives the type of the result and the types of the parameters of the declaration of a function. A result whose type is a name of `typedefs` gets the type of that `typedef`. It also gives the types of the parameters as the declaration spells them, for `signed`. |
| `split_top(text)` | Gives the parameters of a list, cut at the commas outside parentheses, with no `void`. |
| `pointed(node)` | Gives the declaration of a variable of a pointer to a function, as `declaration` gives it, or `None` for a different type. |
| `found(root, path, tree, names, callbacks)` | Gives the functions and the pointer variables of `names` that a file uses, as *The rename* tells, and the declaration of each of them. It also gives the end of the definition of each function of `callbacks` that the file defines and uses. |
| `directive(line)` | Gives the name of the directive of a line, an empty text for a blank line, or `None` for a line of code. |
| `included(text)` | Gives the byte after the last `#include` of a file, or after the `#endif` of the `#if` that holds it. |
| `signed(name, declaration, prefix)` | Gives the start of the function with the new name, and the call of the function `name` in it. The new name is `prefix`, then `name`. |
| `placing(text, at)` | Gives the line and the column of a byte. |
| `writer(values)` | Gives the text of the writer of a file, as *The writer* tells. `values` gives the variable, the rest of the line and the array that the macOS constructor finds. |
