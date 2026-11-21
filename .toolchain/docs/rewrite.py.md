# .toolchain/tools/rewrite.py

`rewrite.py` makes a changed copy of the code, for `test --branches`, for `test --leaks` and for the cases of `test` that state `fail`.
It holds the parts that these runs share.
`.toolchain/docs/branches.py.md`, `.toolchain/docs/leaks.py.md` and `.toolchain/docs/fail.py.md` tell what each run writes into the copy.

## The steps

1. `sources` finds each artifact that the build makes for this target.
2. It takes each file `.c` of `src` that such an artifact compiles, with its command `dump`: *The syntax tree*.
   A file gets 1 job for each different command, with no regard to the output of the command. Thus a variant with a different `defines` gets a tree of its own.

   The command of an artifact with `inputs` holds `--embed-dir=<work>.inputs`, the folder of its stand-ins. Each artifact has a different folder.
   Thus 2 artifacts with `inputs` that compile 1 file give 2 jobs, also with the same flags.

   It also keeps the parts of all the artifacts that compile the file. A file of a project with no parts has no part.
3. `each` runs the command of each job, `{jobs}` jobs at the same time: `.toolchain/docs/toolchain.toml.md`, *The jobs*. It gives each syntax tree to the run, and gives the parts of the file with its result.
4. `united` joins the results of `found` for 1 file: the functions that each tree uses, and their declarations.
5. `copied` copies `src` and `include` into `<build>/<run>/tree`, and writes the area: *The area*. The run then writes its changes into the copies.

`test.py` then builds the copies, and runs the cases: `.toolchain/docs/test.py.md`.

## The test sources

A test source is a file `.c` of `tests` that an artifact of the kind `program`, `library` or `archive` compiles.
Only the run of the leaks asks for the test sources: `.toolchain/docs/leaks.py.md`, *What it counts*.
`copied` then copies the whole folder of each test source, so that a copy still finds a header beside it.
A source of a different kind, for example a module of a kind of the project, gets no copy and no count.

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
It copies the counts, and gives them to `c23_counts_keep` of `.toolchain/runtime/counts.h`, which writes the line: `.toolchain/docs/counts.h.md`.
The writer includes `counts.h` by its full path, `RUNTIME`, so the build of the copy needs no flag.
If `C23_LEAKS` is not set, it writes nothing. A wasm module gets no writer: `.toolchain/docs/leaks.py.md`, *The count in a file*.

On macOS, a `[[gnu::constructor]]` of each file opens its own library again, with `RTLD_NOLOAD` and `RTLD_NODELETE`.
Thus a closed library stays mapped, and it does not matter which code opens and closes it, for example a released library.
Then its destructor runs at the exit of the program, with the code of the library still in place.
The constructor finds its library with `dladdr`. It declares `dladdr` and `dlopen` with names of its own, so it includes no header.

## The area

`copied` writes `c23b_area.c` into the folder of the copies. The kind `wasm_module` adds it to each module of a run, with the key `probed`: `.toolchain/docs/presets.md`, *wasm_module*.
It defines `c23b_outcomes`, the area that a host reads, and `c23b_plan`, the plan of `fail` that a host writes: `.toolchain/docs/outcomes.h.md`, *The area*.
The area holds the addresses of the sections `c23b` and `c23l`, which the linker gives as `__start_c23b`, `__stop_c23b`, `__start_c23l` and `__stop_c23l`.
These 4 names are `[[gnu::weak]]`. Thus a module with no chunk in a section gets the address 0 for that section, and links.

## The syntax tree

A kind can state its own command `dump`: `.toolchain/docs/toolchain.toml.md`, *[kinds.\<kind\>]*. A kind with no `dump` uses `[rewrite] dump`.

`dump` runs `zig cc` with `-fsyntax-only`, and with the other flags of the artifact.
zig then writes an error ending with `FileNotFound`, because no object is made. `dumped` ignores only such an error.
If clang writes a different error, `dumped` stops: `the tree of <path> cannot be read:`, and the errors.
If the output is not JSON, it stops: `the tree of <path> is not JSON`.

`dumped` gives the output of the command to a file of its own in the folder of `<work>`. It reads the tree from that file, and removes it. Thus no pipe holds a tree.

The tree holds each header that the file includes, for example `windows.h`. Thus the tree of 1 file can be much larger than the file.
`parsed` reads that text in parts of 16 MiB, and reads 1 node of the top of the tree at a time. It keeps only these nodes:

- each node of the top that is in `src`, `include` or `tests`, whole;
- each declaration of a function, a variable or a `typedef` in a different file, with only its type and the types of its parameters.

`found` reads these declarations. Thus the memory of a tree holds mostly the code of the project, and not the headers of the system.
The dump writes the file of a location only when it changes. Thus `located` gives each location its file while `parsed` reads the text, in the order of the text.

A node can continue into the next part. `parsed` then reads the next part, and reads the node again from its start, with the file of the location before the node.
A tree that ends before the `]` of the top is not JSON.
If the machine has no memory for a tree, it stops: `the tree of <path> needs more memory than this machine has free`.

The dump sets `_FORTIFY_SOURCE` to 0. Then a call that the system would fortify, such as `snprintf`, does not read as a macro.
Without that macro, the headers of macOS declare a function such as `sprintf` as deprecated, which the compile of the file does not see.
Thus the dump has `-Wno-deprecated-declarations`: the dump reads the tree, and only the compile judges a deprecated function.

The run builds the files of `inputs` after the dump, so they do not exist yet.
zig gives an `#embed` of a file that does not exist no bytes and no error, and an empty array is an error of `-pedantic-errors`.
Thus `stood_in` writes a file of 1 zero byte for each file of `inputs`, into `<work>.inputs`.
The dump has `--embed-dir=<work>.inputs` in `{inputs}`. The syntax tree needs no byte of an input.
The build of the copies embeds the files that the run builds.

## Limits

- A case that crashes, or that stops before its end, writes no line.
- A type that a declaration spells with a different attribute, for example `vectorcall` or `noreturn`, stops the build of the copy. So does a type name that the end of the file does not know, for example a `typedef` in a block.
- A file that 2 jobs read gets the functions that each tree uses. If a variant does not declare a function that a different variant uses, the build of the copy stops.
- Every file of the count writes with a `[[gnu::destructor]]`. A destructor of a library that `dlclose` unmaps on macOS would run at the exit of the program with its code gone. So on macOS each file keeps its own library mapped, and the destructor runs in place.

## Functions

| Function | Result |
| --- | --- |
| `tree(values, run)` | Gives the folder of the copies of a run, `<build>/<run>/tree`. |
| `located(item, state)` | Gives a location of the syntax tree the file that it is in, as `_file`. The dump writes a file only when it changes, so `state` holds the last file. |
| `is_ours(node, root)` | Tells if a node of the top of the tree is in `src`, in `include` or in `tests`, also through the expansion of a macro. |
| `kept(node, root)` | Gives the node of the top of the tree that `parsed` keeps: *The syntax tree*. It gives `None` for a node that no run reads. |
| `parsed(path, root)` | Reads the syntax tree in the file `path`, 1 node of the top at a time, and keeps the nodes of `kept`. |
| `is_macro(loc)` | Tells if a location comes from a macro. |
| `owned(loc, root, tops)` | Gives the path of a location in `src` or in `include`, or `None`. With `tops` at `None`, it gives the path of a location in any folder of the project. |
| `dumped(command, root)` | Runs the command `dump`, and gives the syntax tree, with the file of each location. |
| `stood_in(artifact, mine)` | Writes a file of 1 zero byte for each file of `inputs`, and gives the flag `--embed-dir` of their folder, or no flag. |
| `sources(config, values, tests)` | Gives a job for each file `.c` of `src` that an artifact of this target compiles, and for each different command `dump` of the file. A job holds its command and the set of the parts of the file. With `tests`, it gives the test sources too: *The test sources*. |
| `each(config, values, use, tests)` | Gives the result of `use(root, path, tree)` and the sorted parts of the file, for each job of `sources`, `{jobs}` jobs at the same time. |
| `united(files)` | Gives 1 result of `found` for each file of `files`: the functions that each of its trees uses, their declarations and their ends. |
| `copied(root, where, more)` | Empties `where`, copies `src` and `include` into it, and writes `c23b_area.c`: *The area*. It also copies the folder of each path of `more` that is not in them. |
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
| `writer(values)` | Gives the text of the writer of a file, as *The writer* tells. `values` gives the path of the file, the size of its line, the number of its counts and the array of the counts. |
