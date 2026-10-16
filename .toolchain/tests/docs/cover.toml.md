# .toolchain/tests/cases/cover.toml

The group `cover` checks `.toolchain/tools/cover.py`.
Each case writes a small project into `{case}/p` with `files`, and runs `cover.py --root {case}/p`.
The project has the header `{case}/p/include/sample.h` and its document `{case}/p/docs/extern/sample.h.md`.
In this document, each path of the small project starts with `{case}/p`.

| Case | Claim | False variant |
| --- | --- | --- |
| a function that its document does not name | The header declares `sample_add`. `cover.py` writes `include/sample.h: sample_add is not in docs/extern/sample.h.md`. | The document names `sample_add`. |
| a function with an attribute | The header declares `[[nodiscard]] int sample_add(int a, int b);`. `cover.py` writes `sample_add is not in`. | The document names `sample_add`. |
| a function static inline | The header defines `sample_twice`. `cover.py` writes `sample_twice is not in`. | The document names `sample_twice`. |
| a macro with arguments | The header defines `SAMPLE_MAX(a, b)`. `cover.py` writes `SAMPLE_MAX is not in`. | The document names `SAMPLE_MAX`. |
| an item of an enum | The `enum` has `SAMPLE_ONE` and `SAMPLE_TWO`, and the document names `SAMPLE_ONE`. `cover.py` writes `SAMPLE_TWO is not in`. | The document names `SAMPLE_TWO` too. |
| a constexpr | The header declares `SAMPLE_SIZE`. `cover.py` writes `SAMPLE_SIZE is not in`. | The document names `SAMPLE_SIZE`. |
| a layout | The layout `sample_point_t` has the field `x`, and the document names `x`. `cover.py` writes `sample_point_t is not in`. | The document names `sample_point_t` too. |
| a field of a layout | The document names `sample_point_t`, but not `x`. `cover.py` writes `x is not in`. | The document names `x` too. |
| a pointer to a function in a layout | The layout `sample_table_t` has the field `run`, a pointer to a function. `cover.py` writes `run is not in`. | The document names `run` too. |
| a layout with no typedef | The header declares `struct sample_pair`. `cover.py` writes `sample_pair is not in`. | The document names `sample_pair`. |
