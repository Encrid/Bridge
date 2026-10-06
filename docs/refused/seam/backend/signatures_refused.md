# What was refused: Signatures

The record of `signatures.md`: `README.md`, *Three layers, three seams*.

## A call takes nothing and answers a status

Of `signatures.md`, *A call takes nothing and answers a status*.

Refused: `instance_get_func` answering an arity and a count of results for Bridge-ASM700 to check, which
is what this had. A backend that could not tell answered -1 and the check was skipped, so the seam
carried a value for "this does not apply". And two numbers could not say "one result, and it is an
i64", so the wasm backend answered 2. The condition that brings it back is exports of more than one
shape that Bridge-ASM700 must tell apart.
