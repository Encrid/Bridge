# .toolchain/tests/cases/prose.toml

The group `prose` checks `.toolchain/tools/prose.py`.
Each case writes its documents into `{case}` with `files`, and runs `prose.py` on `{case}`.
Each project has a `.toolchain/project.toml`. The cases of the readme give it the name `sample`.
The false variant of a case changes a document.

The words of a sentence of these cases are numbers, for example `1 2 3`, thus a reader can count them.

| Case | Claim | False variant |
| --- | --- | --- |
| the documents of this project | `folders` copies the six files of `docs/contributing` and `.toolchain/docs/prose.py.md` of this project into `{case}`. `prose.py` writes `no fault in 7 files`. | A document `a.md` in `docs` has a sentence of 26 words. |
| a sentence of 26 words | `docs/a.md:3: 26 words in a sentence, and 25 is the most`. | 25 words. |
| a step of 21 words | An item of a numbered list: `docs/a.md:3: 21 words in a sentence, and 20 is the most`. | 20 words. |
| a span of code | A sentence of 25 words has a span of code with three words in it. The span is one word, thus there is no fault. | The three words are not in a span. |
| a cell of a table | A row of a table has two cells of 13 words. Each cell is a unit, thus there is no fault. | The row has one cell of 26 words. |
| an item of a list | Two items of 13 words, with no `.`, give no fault. | One item of 26 words. |
| an item on two lines | The second line of an item starts with two spaces, thus the item has 26 words: `docs/a.md:3: 26 words in a sentence, and 25 is the most`. | The second line starts with no space, thus it starts a paragraph. |
| a paragraph of 7 sentences | `docs/a.md:3: 7 sentences in a paragraph, and 6 is the most`. | 6 sentences. |
| a tag that is not in the list | `docs/a.md:3: the tag 'python' is not one of pseudo c toml sh text mermaid`. | The tag `sh`. |
| a title that is not the name | `readme.md:1: the title is not # sample`. | The title is `# sample`. |
| a readme with no description | A section follows the title: `readme.md:1: no description after the title`. | A description follows the title. |
| a description of 120 characters | `readme.md:3: 120 characters in the description, and 119 is the most`. | 119 characters. |
| sections out of order | `readme.md:9: the section Install must come before Usage`. | Install comes before Usage. |
| a section that the readme needs | `readme.md:1: no section License`. | The readme has the section License. |
| a document of .toolchain/docs | `.toolchain/docs/a.md:3: 26 words in a sentence, and 25 is the most`. | 3 words. |
| a project with no name | `[project] name` is empty: `prose.py: .toolchain/project.toml: write the name of the project in [project] name`. | The name is `sample`. |
