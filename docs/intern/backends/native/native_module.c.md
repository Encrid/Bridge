# src/backends/native/native_module.c

On this backend a module is a shared library, so its image is an opened library.
This file opens that library by path, lists the memories a module imports, and frees them.

## Opening a library

`module_compile` takes the path from the source and opens it with `os_lib_open`.
It refuses an empty path, and refuses a library the operating system will not open.
A library has no import section, so `module_list_imports` always names the shared and the private memory.

## Functions

| Function | Result |
| --- | --- |
| `memory_release` | Frees an adopted block. |
| `module_compile` | Opens the library by path. Gives the image, or null with `err` that tells why. |
| `module_list_imports` | Writes up to `max` imports and gives two: the shared memory and the private memory. |
| `module_destroy` | Closes the library and frees the image. |
