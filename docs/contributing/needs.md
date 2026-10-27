# Needs

A need is an aim that each decision of the project meets.
This document gives the needs of the project, and how to weigh a decision against them.

## The needs

| Need | Meaning |
| --- | --- |
| Fast | The work of each round takes the least time it can. Nothing runs in a round that can run before it, at load. A round makes no copy, takes no lock and makes no allocation that its work does not need. |
| Scalable | The cost of a round grows only with the work in it, not with the count of modules, entities or mods. Thus the system keeps up as these counts grow. |
| Modular | Each part is its own project and its own module. It can be built, tested, replaced and used alone. The parts meet only through stated contracts, such as components and the C ABI. |
| Composable | Any set of parts composes in a deployment with no change to a part. |
| Extensible | A user can extend the project as far as they want. No part forces one way to build. |
| Minimal | The least use of resources, and the least API to learn. The project stays as simple as possible, but it never restricts flexibility or forces one way to build. |
| Explicit | Each value and each behavior is stated, by the part that knows it. Nothing has a meaning by its absence. When a part cannot give a stated value, it says so. |
| Portable | The project runs on each platform of the project, and uses what each platform gives. Where a platform lacks a part, a stated fallback runs. No capability of a platform is restricted. |

## How to weigh a decision

1. Write the options of the decision.
2. For each option, write what it gives in the end, its pros and its cons.
3. Weigh each option against each need, in a table with one row for each need.
4. Add the other rules of the project where they apply.
5. Refuse an option that fails a need. If each option fails a need, the conflict between the needs is the question.

## The needs of a project

A project can add needs of its own below the table, each with its meaning.
