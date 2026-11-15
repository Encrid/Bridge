# .toolchain/runtime/sweep.h

`sweep.h` lets a test program refuse the calls of the code one after the other, in 1 run.
The program is in a case that states `sweep`: `docs/contributing/tests.md`, *A sweep*.
An artifact with `test = true` gets the header with `{toolchain}/runtime` in its `include`.

## Functions

| Function | Result |
| --- | --- |
| `sweep_refuse(call, at)` | Starts a new turn. In that turn, the call number `at` of the function `call` fails, and each count of the calls starts again at 0. |
| `sweep_is_refused()` | Tells if a call failed in the turn. |

## A sweep

A sweep is a loop of the test program:

```c
unsigned at = 0;

do {
    at++;
    sweep_refuse("malloc", at);
    run_the_code();
} while (sweep_is_refused());
```

The first turn refuses the first `malloc` of the code, the second turn the second one.
The loop ends at the turn in which the code makes fewer calls than `at`: then no call failed, and the code ran whole.
Thus 1 run of the program reaches the path of the error of each `malloc` of the code.

## How it works

`sweep_refuse` writes the variable `C23_FAIL` of the environment: ` turn=<turn> <call>=<at>`. It removes the variable `C23_FAILED`.
The layer of `fail.py` reads the turn at each call. A new turn makes it read the numbers again, and start its counts again: `.toolchain/docs/fail.py.md`, *The turn*.
`sweep_is_refused` reads the variable `C23_FAILED`, which the layer sets for a failed call of the turn.

The variables of the environment reach each library of the program. Thus a sweep also refuses the calls of a shared library.
A program calls `sweep_refuse` from 1 source file only: the count of the turns is in that file.
On a build with no layer, no call fails, and the loop ends after its first turn.
