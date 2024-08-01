#define ASM

.globl iret_exec
.globl keyboard_int_wrapper, rtc_int_wrapper, pit_int_wrapper
.globl system_call_wrapper

#include "syscall.h"

#define save_context()  \
    pushl %fs;          \
    pushl %es;          \
    pushl %ds;          \
    pushl %eax;         \
    pushl %ebp;         \
    pushl %edi;         \
    pushl %esi;         \
    pushl %edx;         \
    pushl %ecx;         \
    pushl %ebx;         \

#define restore_context()\
    popl %ebx;          \
    popl %ecx;          \
    popl %edx;          \
    popl %esi;          \
    popl %edi;          \
    popl %ebp;          \
    popl %eax;          \
    popl %ds;           \
    popl %es;           \
    popl %fs;           \

#define restore_context_syscall()\
    popl %ebx;          \
    popl %ecx;          \
    popl %edx;          \
    popl %esi;          \
    popl %edi;          \
    popl %ebp;          \
    addl $4, %esp;      \
    popl %ds;           \
    popl %es;           \
    popl %fs;           \

syscall_entries:
    .long 0
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    .long set_handler
    .long sigreturn
    .long create
    .long delete

/*
 * iret instruction equivalent to:
 *  pop %eip
 *  pop %cs
 *  popfl
 *  pop %esp
 *  pop %ss
 */
iret_exec:
    iret

keyboard_int_wrapper:
    save_context()
    call keyboard_handler
    restore_context()
    iret

rtc_int_wrapper:
    save_context()
    call rtc_handler
    restore_context()
    iret

pit_int_wrapper:
    save_context()
    call pit_handler
    restore_context()
    iret

bad_sysc_num:
    movl $-1, %eax
    jmp system_call_done

system_call_wrapper:
    save_context()
            
    cmpl $1, %eax   /* checks the interrupt number */
    jb bad_sysc_num
    cmpl $12, %eax
    ja bad_sysc_num

    pushw $0x18     /* movw $0x18, %ds */
    popw %ds        /* %ds cannot be directly assigned by immediate */

    pushl %edx
    pushl %ecx
    pushl %ebx
    call *syscall_entries(, %eax, 4)
    addl $12, %esp

system_call_done:
    restore_context_syscall()
    iret
