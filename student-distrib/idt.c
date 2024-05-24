#include "idt.h"
#include "x86_desc.h"
#include "keyboard.h"
#include "rtc.h"

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

void exception_divide_by_zero() {
    cli();
    clear();
    printf(" Exception 0x00: Divide By Zero");
    while (1);
}

void exception_debug() {
    cli();
    clear();
    printf(" Exception 0x01: Debug");
    while (1);
}

void exception_non_maskable_interrupt(){
    cli();
    clear();
    printf(" Exception 0x02: Non-maskable interrupt");
    while(1);
}

void exception_breakpoint(){
    cli();
    clear();
    printf(" Exception 0x03: Breakpoint");
    while(1);
}

void exception_overflow(){
    cli();
    clear();
    printf(" Exception 0x04: Overflow");
    while(1);
}

void exception_bound_range_exceeded(){
    cli();
    clear();
    printf(" Exception 0x05: Bound Range Exceeded");
    while(1);
}

void exception_invalid_opcode(){
    cli();
    clear();
    printf(" Exception 0x06: Invalid Opcode");
    while(1);
}

void exception_device_not_available(){
    cli();
    clear();
    printf(" Exception 0x07: Device Not Available");
    while(1);
}

void exception_double_fault(){
    cli();
    clear();
    printf(" Exception 0x08: Double Fault");
    while(1);
}

void exception_segment_overrun(){
    cli();
    clear();
    printf(" Exception 0x09: Coprocessor Segment Overrun");
    while(1);
}

void exception_invalid_tss(){
    cli();
    clear();
    printf(" Exception 0x0A: Invalid TSS");
    while(1);
}

void exception_segment_not_present(){
    cli();
    clear();
    printf(" Exception 0x0B: Segment Not Present");
    while(1);
}

void exception_stack_segment_fault(){
    cli();
    clear();
    printf(" Exception 0x0C: Stack-Segment Fault");
    while(1);
}
void exception_general_protection(){
    cli();
    clear();
    printf(" Exception 0x0D: General Protection");
    while(1);
}
void exception_page_fault(){
    cli();
    clear();
    uint32_t page_fault_linear_addr;
    asm volatile (
        "movl %%cr2, %0" : "=r" (page_fault_linear_addr)
                         :
    );
    printf(" Exception 0x0E: Page Fault (at 0x%x)", page_fault_linear_addr);
    while(1);
}

void exception_reserved(){
    cli();
    clear();
    printf(" Exception 0x0F: Reserved");
    while(1);
}

void exception_x87_floating_point_error(){
    cli();
    clear();
    printf(" Exception 0x10: x87 FPU Floating-Point Error");
    while(1);
}

void exception_alignment_check(){
    cli();
    clear();
    printf(" Exception 0x11: Alignment Check");
    while(1);
}

void exception_machine_check(){
    cli();
    clear();
    printf(" Exception 0x12: Machine Check");
    while(1);
}

void exception_SIMD_floating_point(){
    cli();
    clear();
    printf(" Exception 0x13: SIMD Floating-Point");
    while(1);
}

void system_call() {
    cli();
    clear();
    printf("System call ^_^");
    while(1);
}

extern void keyboard_intr();
extern void rtc_intr();

void mystery() {
    asm volatile (
        "keyboard_intr:\n"
        "pushfl\n"
        "pushal\n"
        "call keyboard_handler\n"
        "popal\n"
        "popfl\n"
        "iret\n"
        
        "rtc_intr:\n"
        "pushfl\n"
        "pushal\n"
        "call rtc_handler\n"
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
        :
        :
        : "%al", "memory"
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

    INIT_INTERRUPT(KEYBOARD_INTR_INDEX, keyboard_intr);
    INIT_INTERRUPT(RTC_INTR_INDEX, rtc_intr);
    
    INIT_SYSTEMCALL(0x80, system_call);
    
    lidt(idt_desc_ptr);
    return;

}
