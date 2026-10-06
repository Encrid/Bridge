# What was refused: Stage 11

What was weighed in `STAGE11.md` and refused, each with the condition that would bring it back.

## The questions

Of `STAGE11.md`, *The questions*.

Refused: a component's storage layout stated in the manifest. It is the same statement moved
away from the code that depends on it, with nothing to hold the two together. No condition
brings it back.

Refused: stated by the deployment, alone or over the author. The deployment does know how many
entities carry a component; and the module's reading code is compiled for one storage layout, so
another at run time means code for all five in every module, or a read through a call. The
condition that brings it back is reads that go through a call, which `query.md`, *The interface
is a query*, refuses.
