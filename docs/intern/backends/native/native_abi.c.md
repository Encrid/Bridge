# src/backends/native/native_abi.c

`backend_get_abi` gives the seam number that this backend was compiled for.
The entry sits in its own file, so a test can link the backend with another answer.

## Functions

| Function | Result |
| --- | --- |
| `backend_get_abi` | Gives `BACKEND_ABI`. Bridge asks for it first and rejects a library that returns another number. |
