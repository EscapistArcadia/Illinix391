#include "idt.h"
#include "keyboard.h"
#include "rtc.h"
#include "syscall.h"

#define PIT_INTR_INDEX 0x20

#define INIT_IDT_UNPRESENT(i) do {                      \
    idt[i].present = 0;                                 \
} while (0)                                             \

#define INIT_IDT_PRESENT(i, ddppll, type, entry) do {   \
    idt[i].present = 1;                                 \
    idt[i].dpl = ddppll;                                \
    idt[i].reserved0 = 0;                               \
    idt[i].size = 1;                                    \
    idt[i].reserved1 = 1;                               \
    idt[i].reserved2 = 1;                               \
    idt[i].reserved3 = type;                            \
    idt[i].reserved4 = 0;                               \
    idt[i].seg_selector = KERNEL_CS;                    \
    SET_IDT_ENTRY(idt[i], entry);                       \
} while (0)

#define INIT_INTERRUPT(i, entry) INIT_IDT_PRESENT(i, 0, 0, entry)
#define INIT_EXCEPTION(i, entry) INIT_IDT_PRESENT(i, 0, 1, entry)
#define INIT_SYSTEMCALL(i, entry) INIT_IDT_PRESENT(i, 3, 1, entry)

extern void keyboard_int_wrapper();
extern void rtc_int_wrapper();
extern void pit_int_wrapper();
extern void system_call_wrapper();

void *syscall_entries[] = {
    NULL,
    halt,
    execute,
    read,
    write,
    open,
    close,
    getargs,
    vidmap,
    set_handler,
    sigreturn,
    create,
    delete
};

uint8_t exception_occurred = 0;

void exception_divide_by_zero() {
    printf(" Exception 0x00: Divide By Zero\n");
    exception_occurred = 1;
    halt(255);
}

void exception_debug() {
    printf(" Exception 0x01: Debug\n");
    exception_occurred = 1;
    halt(255);
}

void exception_non_maskable_interrupt(){
    printf(" Exception 0x02: Non-maskable interrupt\n");
    exception_occurred = 1;
    halt(255);
}

void exception_breakpoint(){
    printf(" Exception 0x03: Breakpoint\n");
    exception_occurred = 1;
    halt(255);
}

void exception_overflow(){
    printf(" Exception 0x04: Overflow\n");
    exception_occurred = 1;
    halt(255);
}

void exception_bound_range_exceeded(){
    printf(" Exception 0x05: Bound Range Exceeded\n");
    exception_occurred = 1;
    halt(255);
}

void exception_invalid_opcode(){
    printf(" Exception 0x06: Invalid Opcode\n");
    exception_occurred = 1;
    halt(255);
}

void exception_device_not_available(){
    printf(" Exception 0x07: Device Not Available\n");
    exception_occurred = 1;
    halt(255);
}

void exception_double_fault(){
    printf(" Exception 0x08: Double Fault\n");
    exception_occurred = 1;
    halt(255);
}

void exception_segment_overrun(){
    printf(" Exception 0x09: Coprocessor Segment Overrun\n");
    exception_occurred = 1;
    halt(255);
}

void exception_invalid_tss(){
    printf(" Exception 0x0A: Invalid TSS\n");
    exception_occurred = 1;
    halt(255);
}

void exception_segment_not_present(){
    printf(" Exception 0x0B: Segment Not Present\n");
    exception_occurred = 1;
    halt(255);
}

void exception_stack_segment_fault(){
    printf(" Exception 0x0C: Stack-Segment Fault\n");
    exception_occurred = 1;
    halt(255);
}
void exception_general_protection(){
    printf(" Exception 0x0D: General Protection\n");
    exception_occurred = 1;
    halt(255);
}
void exception_page_fault(){
    uint32_t page_fault_linear_addr;
    asm volatile (
        "movl %%cr2, %0" : "=r" (page_fault_linear_addr)
                         :
    );
    printf(" Exception 0x0E: Page Fault (at 0x%x)\n", page_fault_linear_addr);
    exception_occurred = 1;
    halt(255);
}

void exception_reserved(){
    printf(" Exception 0x0F: Reserved\n");
    exception_occurred = 1;
    halt(255);
}

void exception_x87_floating_point_error(){
    printf(" Exception 0x10: x87 FPU Floating-Point Error\n");
    exception_occurred = 1;
    halt(255);
}

void exception_alignment_check(){
    printf(" Exception 0x11: Alignment Check\n");
    exception_occurred = 1;
    halt(255);
}

void exception_machine_check(){
    printf(" Exception 0x12: Machine Check\n");
    exception_occurred = 1;
    halt(255);
}

void exception_SIMD_floating_point(){
    printf(" Exception 0x13: SIMD Floating-Point\n");
    exception_occurred = 1;
    halt(255);
}

void system_call0() {
    printf("System call ^_^\n");
    exception_occurred = 1;
    halt(255);
}

/**
 * @brief the meaningless wrapper of wrappers of interrupt handlers
 * to records general-purposed registers and the flags as iterrupts
 * or system call fire
 */
void mystery() {
    asm volatile (
        "keyboard_int_wrapper:\n"
        "pushfl\n"
        "pushal\n"
        "call keyboard_handler\n"
        "popal\n"
        "popfl\n"
        "iret\n"
        
        "rtc_int_wrapper:\n"
        "pushfl\n"
        "pushal\n"
        "call rtc_handler\n"
    // test rtc interrupt, not used
    // );
    // outb(0x0C, 0x70);
    // inb(0x71);
    // asm volatile (
    //     "call test_interrupts\n"
    //     "pushl $8\n"
    //     "call send_eoi\n"
    //     "add $4, %%esp\n"
        "popal\n"
        "popfl\n"
        "iret\n"

        "pit_int_wrapper:\n"
        "pushfl\n"
        "pushal\n"
        "call pit_handler\n"
        "popal\n"
        "popfl\n"
        "iret\n"
        :
        :
        : "%al", "memory"
    );

    /**
     * @brief the handler of system call (IDT entry 0x80)
     * 
     * @param eax the system call number/index
     * @param ebx first parameter
     * @param ecx second parameter
     * @param edx third parameter
     * @return 0 if success, 1 if fail
     */
    asm volatile (
        "bad_sysc_num:\n"
        "movl $-1, %%eax\n"
        "jmp syscall_end\n"
        
        "system_call_wrapper:\n"
        "pushw %%fs\n"      /* records the context */
        "pushw $0\n"
        "pushw %%es\n"
        "pushw $0\n"
        "pushw %%ds\n"
        "pushw $0\n"
        "pushl %%eax\n"
        "pushl %%ebp\n"
        "pushl %%edi\n"
        "pushl %%esi\n"
        "pushl %%edx\n"
        "pushl %%ecx\n"
        "pushl %%ebx\n"
        
        "cmpl $1, %%eax\n"  /* checks the interrupt number */
        "jb bad_sysc_num\n"
        "cmpl $12, %%eax\n"
        "ja bad_sysc_num\n"

        "pushw $0x18\n"     /* movw $0x18, %ds */
        "popw %%ds\n"       /* %ds cannot be directly assigned by immediate */

        "shll $2, %%eax\n"  /* %eax *= 4*/
        "addl %0, %%eax\n"  
        "call *(%%eax)\n"   /* %eax = (syscall_entries + %eax * 4)*/

        "syscall_end:\n"
        "popl %%ebx\n"      /* restores the context */
        "popl %%ecx\n"
        "popl %%edx\n"
        "popl %%esi\n"
        "popl %%edi\n"
        "popl %%ebp\n"
        "addl $6, %%esp\n"  /* ignores %eax */
        "popw %%ds\n"
        "addl $2, %%esp\n"
        "popw %%es\n"
        "addl $2, %%esp\n"
        "popw %%fs\n"
        "iret"
        :
        : "g"(syscall_entries)
        : "eax", "ebx", "ecx", "edx", "esi", "edi"
    );
}

/**
 * @brief initializes IDT for the OS and loads it to IDTR
 */
void idt_init() {
    int i;

    INIT_EXCEPTION(0x00, exception_divide_by_zero);
    INIT_EXCEPTION(0x01, exception_debug);
    INIT_EXCEPTION(0x02, exception_non_maskable_interrupt);
    INIT_EXCEPTION(0x03, exception_breakpoint);
    INIT_EXCEPTION(0x04, exception_overflow);
    INIT_EXCEPTION(0x05, exception_bound_range_exceeded);
    INIT_EXCEPTION(0x06, exception_invalid_opcode);
    INIT_EXCEPTION(0x07, exception_device_not_available);
    INIT_EXCEPTION(0x08, exception_double_fault);
    INIT_EXCEPTION(0x09, exception_segment_overrun);
    INIT_EXCEPTION(0x0A, exception_invalid_tss);
    INIT_EXCEPTION(0x0B, exception_segment_not_present);
    INIT_EXCEPTION(0x0C, exception_stack_segment_fault);
    INIT_EXCEPTION(0x0D, exception_general_protection);
    INIT_EXCEPTION(0x0E, exception_page_fault);
    INIT_EXCEPTION(0x0F, exception_reserved);
    INIT_EXCEPTION(0x10, exception_x87_floating_point_error);
    INIT_EXCEPTION(0x11, exception_alignment_check);
    INIT_EXCEPTION(0x12, exception_machine_check);
    INIT_EXCEPTION(0x13, exception_SIMD_floating_point);

    for (i = 0x14; i < NUM_VEC; ++i) {
        INIT_IDT_UNPRESENT(i);
    }

    INIT_INTERRUPT(PIT_INTR_INDEX, pit_int_wrapper);
    INIT_INTERRUPT(KEYBOARD_INTR_INDEX, keyboard_int_wrapper);
    INIT_INTERRUPT(RTC_INTR_INDEX, rtc_int_wrapper);
    
    INIT_SYSTEMCALL(0x80, system_call_wrapper);
    
    lidt(idt_desc_ptr);
    return;
}
