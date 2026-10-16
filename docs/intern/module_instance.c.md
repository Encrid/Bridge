# src/module_instance.c

This file compiles the image of a module and makes its instance.
A module gets the memories that it imports, and no other.

## The steps

`module_make_instance` makes a module ready to call, in these steps:

1. Compile the image from the bytes and the path of the artifact, with the demands of the config.
2. Reserve the private memory: `docs/intern/module_call.c.md`, *The private memory*.
3. Write the header of the private memory, with `block_write_header`.
4. Find the memories that the image imports.
5. Make the instance with those memories.
6. Define the eight imports of the ABI: `docs/intern/abi_define.c.md`, *The signatures*.
7. If `memory_limit` is above 0, set it on the instance.

The demands tell the backend which guarantees the config asks for.
A guarantee can cost something at each instruction, and a backend need not compile in a guarantee that the module declined.

## The memories a module imports

Bridge owns the memory of a module, and the backend only makes it addressable: `memory_adopt` takes pages that Bridge already has.
So one block reaches a native module as a pointer, and a wasm module as an import.

Not every module imports both memories.
So Bridge reads the imports of the image, and does not hand over memories that nobody asked for, which would make the link fail.
An import of a memory gets one of the two memories there are:

| Namespace | Name | Memory |
| --- | --- | --- |
| `BRIDGE_NAMESPACE`, `bridge` | `BRIDGE_SHARED`, `shared` | The shared block, as the backend of the module sees it. |
| `BRIDGE_NAMESPACE`, `bridge` | `BRIDGE_PRIVATE`, `private` | The private memory of the module. |

Any other import gets nothing from this file.
Bridge reads at most 64 imports, and hands at most two memories.

## Functions

| Function | Result |
| --- | --- |
| `is_text` | Gives whether `len` bytes at `at` are the text `text`. |
| `lend_memory` | Writes the memory that an import asks for, if it is one of the two there are. Gives how many it wrote. |
| `lend_memories` | Writes the memories that the image imports. Gives how many. |
| `module_make_instance` | Compiles the image of a module, reserves its private memory, and makes its instance with the memories it imports and the ABI. Gives 1 with `err`. |
