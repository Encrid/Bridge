# .toolchain/header.txt

`header.txt` holds the legal notice that starts each file of code.
`shared.header` reads it, and `text.py` and `style.py` check each file against it.

```text
Copyright (C) <year> <holder>

Under the GNU Affero General Public License, version 3 only: see license.
```

- `<year>` is the year of the first version of the file: four digits, or two years with a `-` between them.
- `<holder>` is a holder of the rights in the file.
- The line of the copyright can come more than one time, one line for each holder.
- The other lines are the same in each file.

A file of Python writes each line after `# `, and an empty line as `#`.
A file of C writes the lines between `/*` and `*/`, each line after `** `, and an empty line as `**`.
