flecs 4.1.6, from https://github.com/SanderMertens/flecs, MIT.

Only the amalgamation is here: `distr/flecs.c` and `distr/flecs.h` at the tag `v4.1.6`
(commit fb55f3c25660425cfe1bc4cf5e6bff8b3f18a9b8), two files rather than the whole tree.

    flecs.c   sha256 6005392eb13c0f3c7abdecb2f85271e50934f63919afebf0bbe85f6dfc7320d6
    flecs.h   sha256 526036a5a41678e2a43a3cb835e9eaa70fd1993868b1978950c0d275752f69b1

Vendored on 2026-10-04. **Nothing in these two files has been edited, and nothing may be.**

The build does not compile them. It compiles a patched copy that `tools/patch-flecs.py` writes
into the build directory, because Bridge-ASM700 needs flecs to allocate component columns inside a
block it owns, and no wrapper outside flecs can decide where flecs allocates. The script is the
record of every change; `docs/bridge/flecs.md` is why.

The version number does not identify this source; the hashes above do, and the script checks them.
`docs/bridge/flecs.md` has what was learned by trusting the number.
