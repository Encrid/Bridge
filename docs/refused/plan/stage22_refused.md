# What was refused: Stage 22

What was weighed in `stage22.md` and refused, each with the condition that would bring it back.

## Where a backend is written

Of `stage22.md`, *Where a backend is written*.

Refused: both stay in this tree until the seam for a backend is released. It was the one
recommended: a change to the seam is one commit, the library is tested whole, and the seam is
held against two shapes in one tree. Against it, the library carries wasmtime and is more than
a library. The condition that brings it back is changes to the seam proving too costly across
three repositories while nothing is released.

Refused: both move into a host's repository. Running a module's code is not orchestrating, so
it is not a host's, and another host would take its backends from a host. The condition that
brings it back is none.

Refused: the wasm one moves into a host's repository and the native one stays here. The same,
for one of them. The condition that brings it back is none.

Refused: one repository for the two. Ruled against by Enkrid, who wants one for each backend.
The condition that brings it back is the two proving to change together every time.

## How the library is tested with no backend in its tree

Of `stage22.md`, *How the library is tested with no backend in its tree*.

Refused: the tests take the two backends as they are published. Each is built against this
seam, and the cases here would need them built against the seam of the commit at hand: a
change to the seam could not pass here until the backends followed, and they could not follow
until it was handed. And the tree would run what another project's directory gives it. The
condition that brings it back is none while the seam changes.

Refused: a backend of the tests' own, and the cases of the seam handed as a package that each
backend runs. One set of cases for three users, and a package and a form more for a seam that
is not released. The condition that brings it back is the seam released, and the two copies
found to differ once.
