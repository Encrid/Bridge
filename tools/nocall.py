"""No crossing per access: the function a built module exports has no call in it.

    nocall.py <wasm-tools> <module.wasm> <export>

Reads the module as text and checks the body of the function exported under that name: it
must load or store, and it must not call. An accessor that was merged but not inlined would
be a call here, and one that was never merged would be an import the module still has."""
import re
import subprocess
import sys


def main():
    tools, module, export = sys.argv[1:4]
    text = subprocess.run([tools, "print", module], check=True, capture_output=True,
                          text=True).stdout
    if '(import "acc"' in text:
        sys.exit("FAIL: %s still imports an accessor" % module)
    found = re.search(r'\(export "%s" \(func (\d+)\)\)' % re.escape(export), text)
    if not found:
        sys.exit("FAIL: %s exports no function '%s'" % (module, export))
    start = text.index("(func (;%s;)" % found.group(1))
    end = text.find("\n  (func ", start + 1)
    body = text[start:end if end != -1 else len(text)]
    calls = len(re.findall(r"^\s*call(_indirect)?\b", body, re.M))
    touches = len(re.findall(r"^\s*[fi](32|64)\.(load|store)", body, re.M))
    print("%s: %d loads and stores, %d calls" % (export, touches, calls))
    if calls or not touches:
        sys.exit("FAIL: '%s' has %d calls and %d loads and stores" % (export, calls, touches))


main()
