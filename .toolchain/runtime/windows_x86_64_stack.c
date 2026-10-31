/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "windows_x86_64.h"

[[gnu::naked]]
void ___chkstk_ms(void)
{
    __asm__(
        "pushq %rcx\n"
        "pushq %rax\n"
        "leaq 24(%rsp), %rcx\n"
        "1:\n"
        "cmpq $0x1000, %rax\n"
        "jb 2f\n"
        "subq $0x1000, %rcx\n"
        "orq $0, (%rcx)\n"
        "subq $0x1000, %rax\n"
        "jmp 1b\n"
        "2:\n"
        "subq %rax, %rcx\n"
        "orq $0, (%rcx)\n"
        "popq %rax\n"
        "popq %rcx\n"
        "ret\n");
}
