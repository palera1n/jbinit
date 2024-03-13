.globl _msyscall
.globl _thread_switch
.globl ___error
.align 4

#include <fakedyld/syscalls.h>

_msyscall:
    mov x16, x0
    ldp x0, x1, [sp]
    ldp x2, x3, [sp, 0x10]
    ldp x4, x5, [sp, 0x20]
    ldp x6, x7, [sp, 0x30]
    svc 0x80
    b.cs Lbotch
    ret

_fork:
    mov x16, #SYS_fork
    svc 0x80
    b.cs Lbotch
    cbz x1, Lparent
    // child
    mov w0, #0
    ret

Lbotch:
    stp x0, lr, [sp, #-0x10]!
    bl ___error
    ldp x8, lr, [sp], #0x10
    str x8, [x0]
    mov x0, #-1
    ret

Lparent:
    ret

_thread_switch:
    movn x16, #0x3c
    svc 0x80
    ret
