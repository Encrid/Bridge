# src/backends/native/native_instance.c

This file makes and frees a module instance, and holds the three limits the backend ignores.
An instance carries the entry table, the two memory bases, and the functions found in it.

## The limits kept as nothing

`instance_set_work_limit`, `instance_set_timeout` and `instance_set_memory_limit` do nothing.
The backend reports none of these guarantees, so Bridge never hands it a number to keep.

## Functions

| Function | Result |
| --- | --- |
| `instance_create` | Allocates an instance, fills its table, and points it at the shared and private memory. |
| `instance_destroy` | Frees the functions found in the instance, then the instance. |
| `instance_set_work_limit` | Does nothing. |
| `instance_set_timeout` | Does nothing. |
| `instance_set_memory_limit` | Does nothing. |
