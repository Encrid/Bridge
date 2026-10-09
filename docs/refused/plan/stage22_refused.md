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

## What the two are called

Of `stage22.md`, *What the two are called*.

Refused: `Bridge-Backend-Native` and `Bridge-Backend-Wasm`, the words in full. Longer, for
what the two letters say to whoever knows the library. The condition that brings it back is
another project of the library's whose name would begin with the same two letters.

Refused: `Bridge-Native` and `Bridge-Wasm`. It does not say that they are backends, and leaves
no room for a project of the library's that is not one. The condition that brings it back is
none.

Refused: `Native-Backend` and `Wasm-Backend`. Nothing says whose backend. The condition that
brings it back is none.

## What a test module is written with

Of `stage22.md`, *What a test module is written with*.

Refused: `bridge_abi.h` bare, every call written out in every module. About ten thousand lines
written again and longer, what a case is about buried under the same six moves, and helpers
that would come back one at a time with no owner. The condition that brings it back is the
binding being taken for an SDK by somebody outside.

Refused: the binding for every module. A case about a module with no `init`, or with an `init`
of another signature, could not be written. The condition that brings it back is every such
case leaving with the wasm backend.

## How a case gets an outcome

Of `stage22.md`, *How a case gets an outcome*.

Refused: the program of the tests tells the backend that the next call fails. A way into a
backend beside the seam, state shared between threads and between cases, and the limit the
library set never looked at. The condition that brings it back is an outcome needed at a
moment no module can choose.

Refused: the backend's settings say which call fails, and how. A small language in strings,
and the same blindness to the limits. The condition that brings it back is none.

## What whoever writes a backend is handed

Of `stage22.md`, *What whoever writes a backend is handed*.

Refused: the three headers in the library's package. A host would be handed two seams that are
not its own, where it is said to include one header. The condition that brings it back is a
host that has to compile another seam.

Refused: each backend's repository keeps a copy of the three, made by hand. The seam's own
header in three trees, and a backend built against a copy that is behind answers the same
number and is wrong with nothing said. The condition that brings it back is none.

## What each repository is made from

Of `stage22.md`, *What each repository is made from*.

Refused: from the project that new ones are started from, with this tree's tools for C added.
Its own first stage is not closed, its method is another than the one this code was written
by, and two programs would check one tree. The condition that brings it back is that project's
first stage closed, and then for every project at once.
