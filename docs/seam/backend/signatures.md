# Signatures

What crosses the backend seam as a function: the eight imports Bridge-ASM700 defines, and the one shape it
calls.

## Imports are declared, exports are checked

The two directions are not symmetric, and the asymmetry is the design rather than an oversight.

    Bridge-ASM700 -> a module's imports   eight functions, and their signatures VARY
    a module's exports -> Bridge-ASM700   every one is () -> i32

So **a signature is declared going in and verified coming out.** `define_func` carries one:

    typedef struct {
        const enum backend_type *params;  uint32_t param_count;   /* I32 | I64 */
        const enum backend_type *results; uint32_t result_count;
    } backend_sig;

**A function Bridge-ASM700 provides answers a status beside its results.** Non-zero says the call
that reached it must not go on - `budgets.md`, *What running out does* - and a backend that can
unwind a module does so there: the wasm backend raises a trap. One that cannot lets the call
return, and Bridge-ASM700 fails the invocation when it does.

**An arity would not do, and not as a matter of taste.** Five of the eight entries answer a value -
`declare`, `system` and `observe` a slot, `spawn` an id, `alloc` an offset - and a count of
arguments cannot say so. Nor can it say how wide they are: `attach(id, slot, ptr, len)` has four
parameters of which the first is a 64-bit entity id and the rest are 32-bit, `ptr` being an offset
into the scratch arena on every backend. And wasmtime's own
entry settles it - `wasmtime_linker_define_func` takes `const wasm_functype_t *ty` (`linker.h:137`),
params and results with their widths, which **cannot be constructed from a number**.

**Two types, not four.** Every entry is `I32` or `I64` - offsets, lengths and slots are 32-bit, an
entity id is 64 - and **no ABI entry takes or returns a float.**

**Why the type travels as data rather than as the engine's own.** wasmtime wants a `wasm_functype_t`
and the native backend wants only the pointers, so handing over arrays lets each do its own thing, and
passing wasmtime's type would leak the engine into the seam. It also keeps **one copy of the fact**:
a backend told the signature cannot drift from it, where a backend compiling our ABI's signatures
itself would re-derive what Bridge-ASM700 already holds - the thing `wasm.md`, *The ABI
layouts*, refuses for that header.

**What it buys beyond being implementable.** Core wasm checks an import's shape at link time -
`wasm.md`, *Identity*, for what that catches and what it does not - so a module that declares
`declare` wrongly is **refused at instantiation, by name**. Bind to whatever the module declared
instead and the same module links happily and passes garbage - a load-time refusal traded for
runtime corruption.

`module_imports` is not a second source for this: it answers *which memories to supply*, and nothing
about what shape a function has.

## A call takes nothing and answers a status

    err call(func, int32_t *status);

**Every function Bridge-ASM700 calls has the same signature**, because there are only five kinds of them -
`init`, `shutdown`, `reload`, a system and an observer - and all of them take no arguments and
return an i32. Data never travels through a call here: a system reads its spans and writes its
columns, so there is nothing to pass. No marshalling at this seam, no arity to arrange, and nothing
a backend could get wrong about either.

`status` is what the function returned; `err` is whether it returned at all. A trap is an ordinary
outcome - a module that reads out of bounds, exhausts a system's work limit, or runs past a lifecycle
call's timeout produces one, and the process keeps running - and `failure.md`, *When something
fails*, treats the two alike on purpose.

**The backend is who checks an export, because it is who can see one** - the other half of
*Imports are declared, exports are checked*. `instance_func` answers a function `call` can run, or
refuses and says why. The wasm backend reads the export's type and refuses one that is not
`() -> i32`, at init and not on the first frame: `tests/stage2.c`, case `shape`. The native
backend has a symbol from the loader, which carries no type, and checks nothing: it does not
answer `untrusted`, of which the check is a part - `capabilities.md`, *Capabilities, so the swap
is honest*. What it calls is
not `() -> i32` either: it passes its table - `native.md`, *Lifecycle*. "Takes nothing" is what
Bridge-ASM700 hands a backend to pass on, not the shape of the function a backend runs.

Refused: `instance_func` answering an arity and a count of results for Bridge-ASM700 to check,
which is what this had. A backend that could not tell answered -1 and the check was skipped, so
the seam carried a value for "this does not apply". And two numbers could not say "one result,
and it is an i64", so the wasm backend answered 2. The condition that brings it back is exports
of more than one shape that Bridge-ASM700 must tell apart.
