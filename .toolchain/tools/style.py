# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import fnmatch
import re
from pathlib import Path

import shared

ROOT = shared.arguments().root
CONFIG = shared.config(ROOT)
STYLE = CONFIG['style']

COLUMNS = STYLE['columns']
LINES = STYLE['lines']
FUNCTIONS = STYLE['functions']
ARGUMENTS = STYLE['arguments']
BRANCHES = STYLE['branches']
DEPTH = STYLE['depth']
SNAKE = re.compile(r'[a-z_][a-z0-9_]*\Z')
UPPER = re.compile(r'[A-Z_][A-Z0-9_]*\Z')
CONTROL = ('if', 'while', 'for', 'switch')
KEYWORDS = CONTROL + ('else', 'do', 'return', 'sizeof', 'case', 'default', 'goto', 'break',
                      'continue', 'alignof', 'static_assert', 'typeof')
BASE = (r'void|char|short|int|long|float|double|bool|auto|size_t|ptrdiff_t|signed|unsigned|'
        r'\w+_t|struct\s+\w+|enum\s+\w+|union\s+\w+')
ANY = BASE + r'|[A-Za-z_]\w*'
QUALIFIER = r'static|const|constexpr|volatile|register|thread_local|extern|inline'
DECLARES = re.compile(r'(?:\[\[[^\]]*\]\]\s*)*(?:(?:%s)\s+)*(?:%s)\b(?:\s+(?:%s|const|long|int|char))*'
                      r'(?:[\s*]|const\b)*(?:\(\s*\*\s*)?(?P<name>[A-Za-z_]\w*)\s*'
                      r'(?:\)\s*\([^)]*\)\s*)?(?:\[[^\]]*\]\s*)*(?:=|,|\Z)'
                      % (QUALIFIER, ANY, BASE))
HEADER = re.compile(r'/\*\n' + shared.header('**') + r'\*/\n')
ASSIGNS = re.compile(r'(?<![=!<>+\-*/%&|^])=(?!=)')


class Source:
    def __init__(self, path, text):
        self.path, self.text = path, text
        self.lines = text.split('\n')
        self.comments = []
        self.code = self.silenced()
        self.found = []

    def silenced(self):
        out = []
        pattern = re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\\n])*"|(?<![0-9A-Za-z])(?:u8|[LuU])?\'(?:\\.|[^\'\\\n])*\'',
                             re.S)
        last = 0
        for m in pattern.finditer(self.text):
            out.append(self.text[last:m.start()])
            body = m.group(0)
            if body[0] == '/':
                first = self.text.count('\n', 0, m.start()) + 1
                self.comments.append((first, first + body.count('\n'), body))
                out.append(re.sub(r'[^\n]', ' ', body))
            else:
                quote = body.index(body[-1])
                out.append(body[:quote + 1] + 's' * (len(body) - quote - 2) + body[-1])
            last = m.end()
        out.append(self.text[last:])
        lines = ''.join(out).split('\n')
        self.directives = {}
        carried = False
        for n, line in enumerate(lines):
            if carried or line.lstrip().startswith('#'):
                self.directives[n + 1] = self.lines[n]
                carried = line.rstrip().endswith('\\')
                lines[n] = ' ' * len(line)
        code = '\n'.join(lines)
        code = re.sub(r'extern\s+"s*"\s*\{', lambda m: ' ' * len(m.group(0)), code)
        for n, line in self.directives.items():
            if '__cplusplus' in line and n + 1 <= len(lines) and lines[n].strip() == '}':
                at = sum(len(x) + 1 for x in lines[:n])
                code = code[:at] + ' ' * len(lines[n]) + code[at + len(lines[n]):]
        return code

    def line_of(self, at):
        return self.code.count('\n', 0, at) + 1

    def say(self, rule, line, what, function=None):
        self.found.append((self.path, line, rule, what, function))


def tokens_of(code, start, end):
    return [(m.group(0), m.start()) for m in
            re.compile(r'[A-Za-z_]\w*|\d[\w.]*|"s*"|\'s*\'|\[\[|\]\]|\S').finditer(code, start, end)]


def closing(code, at):
    pairs = {'(': ')', '[': ']', '{': '}'}
    depth = 0
    for i in range(at, len(code)):
        if code[i] in pairs:
            depth += 1
        elif code[i] in pairs.values():
            depth -= 1
            if not depth:
                return i
    return len(code) - 1


def opening(code, at):
    depth = 0
    for i in range(at, -1, -1):
        if code[i] in ')]}':
            depth += 1
        elif code[i] in '([{':
            depth -= 1
            if not depth:
                return i
    return 0


def top_level(text, what):
    depth = 0
    for c in text:
        if c in '([{':
            depth += 1
        elif c in ')]}':
            depth -= 1
        elif c == what and not depth:
            return True
    return False


def constant_in(text):
    head = text.split('=')[0]
    return re.search(r'\b(?:const|constexpr)\b', head.rsplit('*', 1)[-1] if '*' in head else head)


def parameters(text):
    text = text.strip()
    if not text:
        return -1
    if text == 'void':
        return 0
    depth, count = 0, 1
    for c in text:
        if c in '([{':
            depth += 1
        elif c in ')]}':
            depth -= 1
        elif c == ',' and not depth:
            count += 1
    return count


class Function:
    def __init__(self, name, start, params, open_at, close_at, prefix, macro):
        self.name, self.start, self.params = name, start, params
        self.open_at, self.close_at, self.prefix, self.macro = open_at, close_at, prefix, macro


def file_scope(src):
    code = src.code
    functions, statements = [], []
    at = begun = 0
    while at < len(code):
        c = code[at]
        if c == ';':
            statements.append((begun, at))
            begun = at + 1
        elif c == '{':
            end = closing(code, at)
            before = code[begun:at].rstrip()
            if before.endswith(')') and not ASSIGNS.search(before):
                paren = opening(code, begun + len(before) - 1)
                name = re.search(r'([A-Za-z_]\w*)\s*\Z', code[begun:paren])
                if name and name.group(1) not in KEYWORDS:
                    word = name.group(1)
                    macro = UPPER.match(word) is not None
                    params = code[paren + 1:begun + len(before) - 1]
                    functions.append(Function(params.strip() if macro else word,
                                              begun + len(code[begun:at]) - len(code[begun:at].lstrip()),
                                              params, at, end, code[begun:paren], macro))
                    at = end + 1
                    begun = at
                    continue
            at = end + 1
            continue
        at += 1
    return functions, statements


def blank_lines(src, first, last):
    return [n for n in range(first, last + 1) if not src.lines[n - 1].strip()]


def check_lines(src):
    code_lines = src.code.split('\n')
    for n, line in enumerate(src.lines, 1):
        if len(line.expandtabs(8)) > COLUMNS:
            src.say('F3', n, 'the line is %d columns, and %d is the most' % (len(line.expandtabs(8)), COLUMNS))
        if '\t' in line[:len(line) - len(line.lstrip())]:
            src.say('L2', n, 'a tabulation is used for indentation')
        if line != line.rstrip():
            src.say('L3', n, 'the line ends in white space')
        code = code_lines[n - 1]
        if not code.strip():
            continue
        if re.search(r',(?=[^\s])', code):
            src.say('L3', n, 'no space after a comma')
        if re.search(r'\b(?:if|while|for|switch)\(', code) or re.search(r'\breturn[^\s;]', code):
            src.say('L3', n, 'no space after a keyword')
        if re.search(r'\bsizeof\s', code):
            src.say('L3', n, 'sizeof is a unary operator: no space after it, sizeof(x)')
        call = re.search(r'\b([a-z_]\w*)\s+\((?!\s*\*)', code)
        if call and call.group(1) not in KEYWORDS and not re.match(r'(?:%s|%s)\Z' % (BASE, QUALIFIER), call.group(1)):
            src.say('L3', n, "a space between the name '%s' and its parenthesis" % call.group(1))
        if re.search(r'[!~]\s', code) or re.search(r'(?:\+\+|--)\s+[\w(*]', code) or re.search(r'[\w)\]]\s+(?:\+\+|--)', code):
            src.say('L3', n, 'a space between a unary operator and what it is applied to')
        spaced = code.replace('->', '  ')
        for m in re.finditer(r'<<=|>>=|[=!<>+\-*/%&|^]=|&&|\|\||<<|>>|[<>?/%^]|(?<![=!<>+\-*/%&|^])=(?!=)', spaced):
            before, after = spaced[:m.start()], spaced[m.end():]
            if (before.strip() and not before.endswith(' ')) or (after.strip() and not after.startswith(' ')):
                src.say('L3', n, "no space on both sides of '%s'" % m.group(0))
                break
        if re.search(r'[\w\]][+\-*&|][\w(]|\)[+|][\w(]', re.sub(r'\d[eE][+-]\d', 'x', spaced)) and not re.search(r'\+\+|--', spaced):
            if not re.search(r'\(\s*\w[\w\s]*\*+\s*\)', spaced):
                src.say('L3', n, 'a binary operator with no space on both sides')
        if re.search(r'[A-Za-z0-9_]\*+\s+[A-Za-z_(]', code) and re.search(r'\b(?:%s)\*' % BASE, code):
            src.say('V3', n, 'the pointer symbol is attached to the type, and belongs on the name')
        if re.search(r'\b(?:%s)\s+\*+\s+[A-Za-z_]' % BASE, code):
            src.say('V3', n, 'the pointer symbol stands apart from the name it belongs to')


def check_file(src, functions):
    path = Path(src.path)
    notice = HEADER.match(src.text)
    if not notice:
        src.say('G1', 1, 'the file does not start with the legal notice')
    for start, end, _ in src.comments[1 if notice else 0:]:
        near = [f.name for f in functions if src.line_of(f.start) - 1 <= end and start <= src.line_of(f.close_at)]
        src.say('F6', start, 'a comment that is not the legal notice', near[0] if near else None)
    if len(functions) > FUNCTIONS:
        src.say('O3', src.line_of(functions[FUNCTIONS].start),
                'the file has %d functions, and %d is the most' % (len(functions), FUNCTIONS))
    for f in functions:
        after = src.line_of(f.close_at) + 1
        gap = 0
        while after + gap <= len(src.lines) and not src.lines[after + gap - 1].strip():
            gap += 1
        if after + gap <= len(src.lines) and gap != 1:
            src.say('G2', after, '%d empty lines after a function, and it is one' % gap, f.name)
    depth = 0
    for n in sorted(src.directives):
        line = src.directives[n]
        word = re.match(r'\s*#\s*(\w+)', line)
        if not word or src.directives.get(n - 1, '').rstrip().endswith('\\'):
            continue
        if word.group(1) in ('endif', 'else', 'elif', 'elifdef', 'elifndef'):
            depth -= 1
        if len(line) - len(line.lstrip()) != 4 * max(depth, 0) or line.lstrip()[1:2] == ' ':
            src.say('G3', n, 'the directive is not indented to its level, which is %d' % max(depth, 0))
        if word.group(1) in ('if', 'ifdef', 'ifndef', 'else', 'elif', 'elifdef', 'elifndef'):
            depth += 1
        define = re.match(r'\s*#\s*define\s+(\w+)(\(?)\s*(.*)', line)
        if define:
            name, takes, body = define.groups()
            more = n
            while src.directives[more].rstrip().endswith('\\') and more + 1 in src.directives:
                more += 1
                body += ' ' + src.directives[more]
            if not UPPER.match(name):
                src.say('V1', n, "the macro '%s' is not in UPPER_CASE" % name)
            if not takes and re.match(r'[-+(]*(?:\d|"|\')', body.strip()):
                src.say('H3', n, "the macro '%s' is a constant" % name)
            whole = ' '.join(body.replace('\\', ' ').split())
            fields = not takes and whole.startswith('{') and whole.endswith('}')
            if ';' in re.sub(r'"[^"]*"', '', body) and not fields:
                src.say('H3', n, "the macro '%s' is more than one statement" % name)
            if path.suffix == '.c':
                src.say('H1', n, "the macro '%s' is defined in a source file" % name)
    if path.suffix == '.h':
        opened = [src.directives[n] for n in sorted(src.directives)][:2]
        guarded = len(opened) == 2 and re.match(r'\s*#\s*ifndef\s+(\w+)', opened[0]) \
            and re.match(r'\s*#\s*define\s+(\w+)\s*$', opened[1])
        if not guarded and not any('pragma once' in d for d in opened):
            src.say('H2', 1, 'the header is not protected from double inclusion')


def check_outside(src, statements, functions, declared):
    code = src.code
    in_header = src.path.endswith('.h')
    for begun, end in statements:
        text = ' '.join(code[begun:end].split())
        line = src.line_of(begun + len(code[begun:end]) - len(code[begun:end].lstrip()))
        if not text:
            continue
        bare = re.sub(r'\[\[[^\]]*\]\]\s*', '', text)
        if bare.startswith('typedef'):
            last = re.search(r'(\w+)\s*(?:\[[^\]]*\])?\Z', bare)
            name = last if '{' in bare else re.search(r'\(\s*\*\s*(\w+)\s*\)\s*\(', bare) or last
            if name and not name.group(1).endswith('_t'):
                src.say('V1', line, "the type '%s' does not end with _t" % name.group(1))
            if not in_header:
                src.say('H1', line, 'a type is declared in a source file')
        elif re.match(r'(?:struct|enum|union)\b[^=(]*\{', bare):
            if not in_header:
                src.say('H1', line, 'a type is declared in a source file')
        elif re.match(r'static_assert\b', bare):
            pass
        elif re.match(r'[A-Z_][A-Z0-9_]*\s*\(', bare):
            pass
        elif re.search(r'\)\s*\Z', bare) and not ASSIGNS.search(bare):
            if not in_header:
                src.say('H1', line, 'a function is declared in a source file')
        elif DECLARES.match(bare):
            name = DECLARES.match(bare).group('name')
            constant = constant_in(bare)
            if not constant:
                src.say('G4', line, "'%s' is a global variable that is not constant" % name)
            elif not UPPER.match(name):
                src.say('V1', line, "the global constant '%s' is not in UPPER_CASE" % name)
        for m in re.finditer(r'\benum\b[^{;]*\{([^}]*)\}', bare):
            for item in m.group(1).split(','):
                name = item.split('=')[0].strip()
                if name and not UPPER.match(name):
                    src.say('V1', line, "'%s', of an enum, is not in UPPER_CASE" % name)
    for m in re.finditer(r'\(\s*\*\s*\w+\s*\)\s*\(([^()]*)\)', code):
        check_parameters(src, src.line_of(m.start()), m.group(1), None)
    for begun, end in statements:
        text = code[begun:end]
        m = re.search(r'\b([A-Za-z_]\w*)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)\s*\Z', text)
        if m and not ASSIGNS.search(text) and not re.match(r'\s*(?:typedef|static_assert)\b', text) \
                and m.group(1) not in KEYWORDS and not UPPER.match(m.group(1)):
            check_parameters(src, src.line_of(begun + m.start()), m.group(2), m.group(1))
    for f in functions:
        static = re.search(r'\bstatic\b', f.prefix)
        exported = re.search(r'\b\w*(?:EXPORT|_API)\b|\bdllexport\b|\bvisibility\b', f.prefix) or f.macro
        if in_header and not re.search(r'\bstatic\s+inline\b|\binline\s+static\b', f.prefix):
            src.say('H1', src.line_of(f.start), "'%s' is defined in a header and is not static inline" % f.name, f.name)
        if not in_header and re.search(r'\binline\b', f.prefix):
            src.say('H1', src.line_of(f.start), "'%s' is static inline in a source file" % f.name, f.name)
        if not in_header and not static and not exported and f.name != 'main' and f.name not in declared:
            src.say('G5', src.line_of(f.start), "'%s' is used in this file alone, and is not static" % f.name, f.name)


def check_parameters(src, line, params, name):
    count = parameters(params)
    if count < 0:
        src.say('F5', line, 'a function that takes nothing takes void', name)
    elif count > ARGUMENTS:
        src.say('F5', line, '%d arguments, and %d is the most' % (count, ARGUMENTS), name)


def check_function(src, f):
    code = src.code
    first, opens, closes = src.line_of(f.start), src.line_of(f.open_at), src.line_of(f.close_at)

    def say(rule, line, what):
        src.say(rule, line, what, f.name)

    if not SNAKE.match(f.name):
        say('F2', first, "the function '%s' is not in snake_case" % f.name)
    if not f.macro:
        check_parameters(src, first, f.params, f.name)
    if closes - opens - 1 > LINES:
        say('F4', opens, 'the body is %d lines, and %d is the most' % (closes - opens - 1, LINES))
    if src.lines[opens - 1].strip() != '{':
        say('L4', opens, "a function's opening bracket is alone on its line")
    if src.lines[closes - 1].strip() != '}':
        say('L4', closes, "a function's closing bracket is alone on its line")

    tokens = tokens_of(code, f.open_at + 1, f.close_at)
    blocks = [{'level': 1, 'case': 0, 'opened': 0, 'control': 0, 'do': False}]
    state = {'extra': 0, 'chain': {}, 'declared': [], 'coded': False, 'ended': 0}

    def level():
        return blocks[-1]['level'] + blocks[-1]['case'] + state['extra']

    def starts(at, expected):
        n = src.line_of(at)
        line = src.lines[n - 1]
        column = at - (code.rfind('\n', 0, at) + 1)
        if not line[:column].strip():
            if column != 4 * expected:
                say('L2', n, 'indented by %d, and its level is %d' % (column, expected))
        elif n == state['ended']:
            say('L1', n, 'more than one statement on the line')
        return n

    def nested():
        return sum(b['control'] for b in blocks) + state['extra']

    i = 0
    while i < len(tokens):
        word, at = tokens[i]
        if word == '}':
            block = blocks.pop() if len(blocks) > 1 else blocks[0]
            n = src.line_of(at)
            if src.lines[n - 1][:at - (code.rfind('\n', 0, at) + 1)].strip():
                say('L4', n, 'a closing bracket is not first on its line')
            elif at - (code.rfind('\n', 0, at) + 1) != 4 * block['opened']:
                say('L2', n, 'a closing bracket is not at the level of what it closes')
            state['extra'] = 0
            i += 1
            if block['do'] and i < len(tokens) and tokens[i][0] == 'while':
                end = closing(code, tokens[i + 1][1])
                while i < len(tokens) and tokens[i][1] <= end + 1:
                    i += 1
            elif i < len(tokens) and src.line_of(tokens[i][1]) == n and tokens[i][0] != 'else':
                say('L4', n, 'a closing bracket is followed by something that is not an else')
            state['ended'] = 0
            continue
        if word == '{':
            starts(at, level())
            blocks.append({'level': level() + 1, 'case': 0, 'opened': level(), 'control': 0, 'do': False})
            state['extra'] = 0
            i += 1
            continue
        if i + 2 < len(tokens) and tokens[i + 1][0] == ':' and tokens[i + 2][0] != ':' \
                and re.match(r'[A-Za-z_]\w*\Z', word) and word not in ('case', 'default') \
                and state['ended'] != src.line_of(at):
            i += 2
            continue
        if word in ('case', 'default'):
            blocks[-1]['case'] = 0
            starts(at, level())
            blocks[-1]['case'] = 1
            while i < len(tokens) and tokens[i][0] != ':':
                i += 1
            i += 1
            continue
        if word in CONTROL or word in ('else', 'do'):
            mine = level()
            n = starts(at, mine)
            last = at + len(word) - 1
            kind = word
            if word == 'else' and i + 1 < len(tokens) and tokens[i + 1][0] == 'if':
                i += 1
                kind = 'else if'
            if kind != 'else' and kind != 'do':
                last = closing(code, tokens[i + 1][1])
                head = code[tokens[i + 1][1]:last + 1]
                if kind != 'for' and ASSIGNS.search(head):
                    say('L1', n, 'a condition and an assignment on one line')
                if head.count('?') > 1:
                    say('C2', n, 'nested or chained ternaries')
            if word == 'if':
                state['chain'][mine] = 1
            elif word == 'else':
                state['chain'][mine] = state['chain'].get(mine, 1) + 1
                if state['chain'][mine] == BRANCHES + 1:
                    say('C1', n, 'more than %d branchings in one if' % BRANCHES)
            if word != 'else' and nested() + 1 > DEPTH:
                say('C1', n, 'conditional branchings nested %d deep, and %d is the most' % (nested() + 1, DEPTH))
            while i < len(tokens) and tokens[i][1] <= last:
                i += 1
            if i < len(tokens) and tokens[i][0] == '{':
                brace = tokens[i][1]
                if src.line_of(brace) != src.line_of(last):
                    say('L4', src.line_of(brace), 'an opening bracket is at the end of the line it belongs to')
                elif src.lines[src.line_of(brace) - 1].rstrip()[-1:] != '{' and code[brace + 1:code.find('\n', brace)].strip():
                    say('L4', src.line_of(brace), 'an opening bracket is not at the end of its line')
                blocks.append({'level': mine + 1, 'case': 0, 'opened': mine, 'control': 1, 'do': kind == 'do'})
                state['extra'] = 0
                i += 1
            else:
                if i < len(tokens) and src.line_of(tokens[i][1]) == src.line_of(last) and tokens[i][0] != ';':
                    say('L1', n, 'a condition and its statement on one line')
                state['extra'] += 1
            state['coded'] = True
            state['ended'] = 0
            continue
        mine = level()
        n = starts(at, mine)
        end = at
        depth = 0
        while end < f.close_at:
            c = code[end]
            if c in '([{':
                depth += 1
            elif c in ')]}':
                depth -= 1
            elif c == ';' and not depth:
                break
            end += 1
        text = ' '.join(code[at:end].split())
        if text.startswith('goto') and re.match(r'goto\b', text):
            say('C3', n, 'goto')
        declares = DECLARES.match(text) and not re.match(r'(?:return|goto|break|continue)\b', text)
        if declares:
            if len(blocks) > 1 or state['extra'] or state['coded']:
                say('L5', n, 'a variable is declared after the beginning of the function')
            else:
                state['declared'].append(src.line_of(end))
            if top_level(text, ','):
                say('L5', n, 'more than one variable declared on a line')
            name = DECLARES.match(text).group('name')
            if not SNAKE.match(name) and not (UPPER.match(name) and constant_in(text)):
                say('V1', n, "'%s' is not in snake_case" % name)
        elif text:
            state['coded'] = True
            if top_level(text, ','):
                say('L1', n, 'several statements joined by a comma')
            if len(ASSIGNS.findall(re.sub(r'\([^()]*\)|\[[^\]]*\]|\{[^{}]*\}', '', text))) > 1:
                say('L1', n, 'several assignments on one line')
            if text.count('?') > 1:
                say('C2', n, 'nested or chained ternaries')
            elif top_level(text, '?') and not re.search(r'(?<![=!<>])=(?!=)', text) \
                    and not text.startswith('return'):
                say('C2', n, 'a ternary used to control the flow')
        state['extra'] = 0
        state['ended'] = src.line_of(end)
        while i < len(tokens) and tokens[i][1] <= end:
            i += 1

    empty = blank_lines(src, opens + 1, closes - 1)
    allowed = state['declared'][-1] + 1 if state['declared'] and state['coded'] else 0
    for n in empty:
        if n != allowed:
            say('L6', n, 'an empty line inside a function')
    if allowed and allowed not in empty:
        say('L6', allowed, 'no empty line after the declarations')


def check(path, text, declared):
    src = Source(path, text)
    functions, statements = file_scope(src)
    check_lines(src)
    check_file(src, functions)
    check_outside(src, statements, functions, declared)
    for f in functions:
        check_function(src, f)
    return src.found


def exceptions():
    listed = []
    for exception in STYLE.get('exception', []):
        where, _, function = exception['path'].partition(':')
        listed.append([exception['rule'], where, function, 0])
    return listed


def ours():
    return sorted(path for path in shared.ours(ROOT) if path.endswith(('.c', '.h')))


def prototypes():
    names = set()
    for path in ours():
        if path.endswith('.h'):
            src = Source(path, (ROOT / path).read_text(encoding='utf-8'))
            names.update(re.findall(r'\b([A-Za-z_]\w*)\s*\(', src.code))
    return names


PROVES = {
    'F3': 'int a_function_with_a_name_long_enough(void);' + ' ' * 40 + 'int x;\n',
    'F4': 'int f(void)\n{\n' + '    g();\n' * 21 + '}\n',
    'F5': 'int f(int a, int b, int c, int d, int e);\n',
    'F6': 'int f(void)\n{\n    /* why */\n    return (0);\n}\n',
    'L1': 'int f(int a)\n{\n    if (a) return (1);\n    return (0);\n}\n',
    'L2': 'int f(void)\n{\n  return (0);\n}\n',
    'L3': 'int f(int a,int b);\n',
    'L4': 'int f(int a)\n{\n    if (a)\n    {\n        g();\n    }\n    return (0);\n}\n',
    'L5': 'int f(void)\n{\n    int a = 1, b = 2;\n\n    return (a + b);\n}\n',
    'L6': 'int f(void)\n{\n    g();\n\n    return (0);\n}\n',
    'V1': 'typedef int age;\n',
    'V3': 'int f(char* name);\n',
    'C1': 'int f(int a)\n{\n    while (a)\n        for (;;)\n            if (a)\n                g();\n    return (0);\n}\n',
    'C2': 'int f(int a)\n{\n    a ? g() : h();\n    return (0);\n}\n',
    'C3': 'int f(void)\n{\n    goto out;\n    return (0);\n}\n',
    'G4': 'int counter;\n',
    'H3': '#define LOOP_COUNTER 0\n',
    'F2': 'static inline int doIt(void)\n{\n    return (0);\n}\n',
    'G2': 'static inline int f(void)\n{\n    return (0);\n}\nstatic inline int g(void)\n{\n    return (1);\n}\n',
    'G3': '#ifdef A\n#define B(x) (x)\n#endif\n',
    'O3': 'static inline int f(void)\n{\n    return (0);\n}\n\n' * 6,
}
PROVES_IN_C = {
    'G5': 'int f(void)\n{\n    return (0);\n}\n',
    'H1': 'typedef int age_t;\n',
}
CLEAN = ('static int add_them(int first, int second)\n{\n    int sum = first + second;\n\n'
         '    if (sum < 0)\n        return (0);\n    for (int i = 0; i < 2; i++) {\n        sum += i;\n    }\n'
         '    return (sum);\n}\n')


def self_test():
    wrong = []
    for path, proves in (('x.h', PROVES), ('x.c', PROVES_IN_C)):
        for rule, text in proves.items():
            if rule not in {f[2] for f in check(path, text, set())}:
                wrong.append('%s is not found in what infringes it' % rule)
    stray = [f for f in check('x.h', CLEAN, set()) if f[2] not in ('G1', 'H2', 'H1')]
    if stray:
        wrong.append('clean C is reported: %s' % stray)
    return wrong


def main():
    wrong = self_test()
    if wrong:
        shared.die('the checker itself: %s' % '; '.join(wrong))
    paths = ours()
    declared = prototypes()
    listed = exceptions()
    found = []
    for path in paths:
        found += check(path, (ROOT / path).read_text(encoding='utf-8'), declared)
    kept = []
    for path, line, rule, what, function in found:
        excused = False
        for entry in listed:
            if entry[0] == rule and fnmatch.fnmatch(path, entry[1]) and entry[2] in ('', function or ''):
                entry[3] += 1
                excused = True
        if not excused:
            kept.append((path, line, rule, what))
    for entry in listed:
        if not entry[3]:
            kept.append(('.toolchain/project.toml', 1, entry[0], "the exception for '%s' excuses nothing" % entry[1]))
    shared.report(['%s:%d: %s %s' % fault for fault in sorted(kept)], len(paths))


if __name__ == '__main__':
    main()
