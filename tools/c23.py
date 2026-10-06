"""No older spelling where C23 has one: docs/design/build.md, *The standard*.

    c23.py

Reads everything of Bridge-ASM700's own - the source, the backends, the headers, the tests and
their modules - and fails on a spelling C23 replaced. Comments and string literals are not
read: a document may say NULL. A backend's dependency is not ours and is not read either.

A macro stays where the preprocessor or a string literal is what is needed, so what is refused
of #define is a number with a name, which is a constexpr.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OURS = ('src/*.c', 'src/*.h', 'backends/*/*.c', 'backends/*/*.h', 'backends/*/sdk/*.h', 'include/*.h',
        'tests/*.c', 'tests/*.h', 'tests/modules/*.h', 'tests/modules/wasm/*.c',
        'tests/modules/native/*.c')

# what is found, and what C23 writes in its place
OLDER = [
    (r'\bNULL\b', 'nullptr'),
    (r'#\s*include\s*<stdbool\.h>', 'nothing: bool, true and false are keywords'),
    (r'\b_Bool\b', 'bool'),
    (r'\b_Alignof\b', 'alignof'),
    (r'\b_Alignas\b', 'alignas'),
    (r'\b_Static_assert\b', 'static_assert'),
    (r'\b_Thread_local\b|\bTHREAD_LOCAL\b', 'thread_local'),
    (r'\b_Noreturn\b', '[[noreturn]]'),
    (r'\b__attribute__\b|\b__declspec\b', 'an attribute in brackets: [[gnu::...]], [[clang::...]]'),
    (r'\benum\s*\{', 'a named enum with its width: enum name : uint32_t {'),
    (r'\benum\s+\w+\s*\{', 'the enum with its width stated: enum name : uint32_t {'),
    (r'\(void\)\s*[A-Za-z_]\w*\s*;', '[[maybe_unused]] where the parameter is declared'),
    (r'=\s*\{\s*0\s*\}\s*;', '= {}'),
    (r'^[ \t]*#[ \t]*define[ \t]+\w+[ \t]+[-+(]*(?:0[xX][0-9a-fA-F]|[0-9])', 'a typed constexpr'),
]

# a comment, or a string or character literal: blanked, with its line breaks kept
SILENT = re.compile(r'''/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\\n])*"|'(?:\\.|[^'\\\n])*\'''', re.S)


def main():
    found = 0
    for pattern in OURS:
        for path in sorted(ROOT.glob(pattern)):
            text = path.read_text(encoding='utf-8')
            code = SILENT.sub(lambda m: re.sub(r'[^\n]', ' ', m.group(0)), text)
            for older, newer in OLDER:
                for m in re.finditer(older, code, re.M):
                    line = code.count('\n', 0, m.start()) + 1
                    print('%s:%d: %s  ->  %s' % (path.relative_to(ROOT).as_posix(), line,
                                                 m.group(0).strip(), newer))
                    found += 1
    if found:
        sys.exit('FAIL: %d older spellings' % found)
    print('no older spelling in what is ours')


if __name__ == '__main__':
    main()
