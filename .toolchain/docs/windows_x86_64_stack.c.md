# .toolchain/runtime/windows_x86_64_stack.c

This file gives `___chkstk_ms` to each shared library of `x86_64-windows-gnu`: `.toolchain/docs/windows_x86_64.h.md`.

## The probe

Windows grows the stack of a thread one page at a time, when the code touches the guard page under the stack.
A function whose frame is larger than a page could jump over the guard page, and the program would stop.
So the compiler calls `___chkstk_ms` before such a function makes its frame, with the size of the frame in `rax`.

`___chkstk_ms` touches each page of 4096 bytes from the stack pointer down, then the last address of the frame.
It changes no value: each touch is an `or` with 0, which writes back the value that it read.
It keeps `rax` and `rcx`, and it does not move the stack pointer, as the compiler needs.
The function is naked: the compiler adds no code before or after its assembly, so the assembly runs as written.

## Functions

| Function | Result |
| --- | --- |
| `___chkstk_ms` | Touches each page of a frame of `rax` bytes under the stack pointer. |
