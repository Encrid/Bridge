# .toolchain/runtime/windows_x86_64.h

This header declares the runtime of the target `x86_64-windows-gnu`: 6 functions of the C library that each shared library of that target gives itself.
`runtime` of `[targets.x86_64-windows-gnu]` adds its sources to each shared library: `.toolchain/docs/toolchain.toml.md`, *[targets.\<target\>]*.

## Why

On `x86_64-windows-gnu`, zig builds compiler_rt as one object of 225 functions, about 47 KB of code.
The start-up code of mingw-w64 needs 6 of them: `___chkstk_ms`, `memcpy`, `memmove`, `memset`, `memcmp` and `strlen`.
The linker cannot take a part of one object, so each shared library got the whole object.
When the library gives these 6 functions itself, the linker takes no compiler_rt.
On the other targets, compiler_rt adds almost nothing to a library, so they need no runtime.

## The files

| File | Functions |
| --- | --- |
| `.toolchain/runtime/windows_x86_64_move.c` | `memcpy` and `memmove` |
| `.toolchain/runtime/windows_x86_64_fill.c` | `memset` |
| `.toolchain/runtime/windows_x86_64_compare.c` | `memcmp` and `strlen` |
| `.toolchain/runtime/windows_x86_64_stack.c` | `___chkstk_ms` |

## The header

| Item | Content |
| --- | --- |
| `SHORT_PATH` | 512 bytes. A copy or a fill of this size or more goes to Windows, which does it faster. |
| `word8_t`, `word4_t`, `word2_t` | Integers of 8, 4 and 2 bytes with no alignment, so a function can move them at any address. |
| `move_memory` | `RtlMoveMemory` of `kernel32.dll`, under a name in snake case. |
| `fill_memory` | `RtlFillMemory` of `kernel32.dll`, under a name in snake case. |

Each function of the runtime has the attribute `[[clang::no_builtin]]`.
Without it, clang can turn a loop of the function into a call to `memcpy` or `memset`, and the function then calls itself.

## The speed

These times are of one call, measured on a Ryzen 5 7640U, against the functions of compiler_rt:

| Size | `memcpy` | `memset` |
| --- | --- | --- |
| 16 bytes | 1.7 ns, against 1.9 ns | 1.6 ns, against 1.4 ns |
| 64 bytes | 1.7 ns, against 2.7 ns | 1.8 ns, against 2.1 ns |
| 256 bytes | 3.7 ns, against 3.9 ns | 3.7 ns, against 3.5 ns |
| 4 KB | 55 ns, the same | 30 ns, against 55 ns |
| 1 MB | 17 us, the same | 7.6 us, against 14 us |

So `memset` of 16 bytes and of 256 bytes is about 0.2 ns slower. Each other size is the same or faster.
A shared library is about 11 KB, against about 72 KB with compiler_rt.
