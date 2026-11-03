# Timings

This document tells how to time a commit that must keep the speed of Bridge.

## The step

Each refactor commit and each perf commit gets a timing against its parent before its review:

```sh
python tools/reference.py --pair <parent> <commit>
```

`docs/tools/reference.py.md` tells how the tool times the 2 trees, and which time is slower.

1. Run the step on an idle machine on AC power, in one power mode for the whole run.
2. Put the report under the heading `## Cases` of the description of the pull request.
3. The reviewer accepts each slower time, or refuses the commit.

`docs/contributing/pull_request.md`, *The description* gives the headings of a description.
A perf commit names the row that shows its gain. If no row times its code, its description says so.

## The layout of the code

Each artifact with `test = true` starts each function at a multiple of 64 bytes.
It also starts each loop there, if zig expects that the loop runs often.
Thus a change of other code does not move the start of such a loop inside its cache line.
The variable `aligned` of each target in `.toolchain/config/targets.toml` holds the flags of zig that do it.
`tools/reference.py` compiles its 3 controls with these flags too.
The shipped library and archive do not use them, so their code does not change.

On `aarch64-windows-gnu`, `aligned` holds only the flag for the functions.
The compiler stops with an error when it aligns a loop for that target: https://github.com/llvm/llvm-project/issues/47432.
Thus on that target, a change of other code can move a loop inside its cache line.
A case of `tests/shared/cases/aligned.toml` fails when the compiler aligns such a loop.
Then that target gets the flag for loops too.
