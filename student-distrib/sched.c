#include "sched.h"
#include "filesys.h"
#include "i8259.h"
#include "syscall.h"
#include "paging.h"
#include "term.h"

#define EXECUTABLE_MAGIC        0x464C457F
#define HIDDEN_PDE_OFFSET       0xBA

#define PIT_FREQUENCY           1193182
#define PIT_SQUARE_MODE         0x36

#define PIT_CHANNEL_0           0x40
#define PIT_CHANNEL_1           0x41
#define PIT_CHANNEL_2           0x42
#define PIT_COMMAND             0x43

extern pcb_t *pcbs[MAX_PROCESS];

void iret_wrapper() {
    asm volatile ("iret_exec: iret");
}

void pit_init(uint16_t frequency) {
    uint16_t param = PIT_FREQUENCY / frequency;

    outb(PIT_SQUARE_MODE, PIT_COMMAND);
    outb((uint8_t)(param & 0xFF), PIT_CHANNEL_0);           /* send the frequency byte by byte */
    outb((uint8_t)((param >> 8) & 0xFF), PIT_CHANNEL_0);    /* shift right and reserve last 8 bytes*/

    enable_irq(0);                          /* enable the interrupt 0x20 in PIC */
}

void initiate_shells() {
    extern file_operations_t stdin_ops;
    extern file_operations_t stdout_ops;

    page_directories[USER_ENTRY].MB.present = 1;
    page_directories[USER_ENTRY].MB.user_supervisor = 1;
    page_directories[USER_ENTRY].MB.read_write = 1;

    page_table_kernel_vidmem[VIDMEM_INDEX + 1].present = 1;     /* 0xB9000: physical 0xB8000 */
    page_table_kernel_vidmem[VIDMEM_INDEX + 1].page_base_address = VIDMEM_INDEX;
    page_table_kernel_vidmem[VIDMEM_INDEX + 2].present = 1;     /* 0xBA000: first terminal */
    page_table_kernel_vidmem[VIDMEM_INDEX + 3].present = 1;     /* 0xBB000: second terminal */
    page_table_kernel_vidmem[VIDMEM_INDEX + 4].present = 1;     /* 0xBC000: third terminal */
    // page_table_kernel_vidmem[VIDMEM_INDEX + 5].present = 1;     /* 0xBD000: fourth terminal */

    int32_t pid;
    uint32_t magic;
    dentry_t de;
    if (read_dentry_by_name((const uint8_t *)"shell", &de) == -1
        || read_data(de.inode_num, 0, (uint8_t *)&magic, sizeof(uint32_t)) == -1
        || magic != EXECUTABLE_MAGIC) {
        printf("Executable shell was not found!");
        return;
    }

    int i;
    uint8_t entry[4];
    pcb_t *pcb;
    for (pid = TERMINAL_COUNT - 1; pid >= 0; --pid) {
        /* initializes termial struct for each terminal */
        terms[pid].pid = pid;
        memset(terms[pid].input.content, 0, MAX_TERMINAL);
        terms[pid].input.length = 0;
        terms[pid].input.in_progress = 0;
        terms[pid].input.to_be_halt = 0;
        terms[pid].cursor.x = 0;
        terms[pid].cursor.y = 0;

        /* initializes pcb for each terminal */
        pcb = pcbs[pid];
        pcb->present = 1;
        pcb->vidmap = 0;
        pcb->rtc = 0;
        pcb->pid = pid;
        pcb->parent = NULL;                                 /* terminal does not have parent */
        pcb->parent_ebp = 0;                                /* never halt terminal */
        pcb->esp0 = KERNEL_STACK - KERNEL_STACK_SIZE * pid; /* stores kernel stack */
        
        pcb->files[0].present = 1;                          /* initiates file descriptor */
        pcb->files[0].ops = &stdin_ops;
        pcb->files[1].present = 1;
        pcb->files[1].ops = &stdout_ops;
        for (i = 2; i < 8; ++i) {
            pcb->files[i].present = 0;
        }

        page_directories[USER_ENTRY].MB.page_base_address = 2 + pid;
        asm volatile (                                      /* flushes the TLB */
            "movl %%cr3, %%eax\n"                           /* copies the shell's image */
            "movl %%eax, %%cr3\n"
            :::"eax"
        );

        read_data(de.inode_num, 24, (uint8_t *)entry, sizeof(entry));  /* gets the entry */
        read_data(de.inode_num, 0, (uint8_t *)PROGRAM_IMAGE, PROGRAM_IMAGE_LIMIT);

        asm volatile (
            "movl %%esp, %%esi\n"
            "movl %5, %%esp\n"              /* prepare iret contexts for all terminals on their stacks */
            "pushl %1\n"                    /* SS */
            "pushl %2\n"                    /* ESP */
            "pushfl\n"                      /* EFLAGS*/
            "pushl %3\n"                    /* CS */
            "pushl %4\n"                    /* EIP */
            "pushl $iret_exec\n"
            "pushl %4\n"
            "movl %%esp, %0\n"              /* pcb->ebp = %esp, for continuing iret! */
            "movl %%esi, %%esp\n"           /* restore esp */
            : "=r"(pcb->ebp)
            : "r"((uint32_t)USER_DS),
              "g"((uint32_t)USER_STACK),
              "g"((uint32_t)USER_CS),
              "r"(((uint32_t)entry[3] << 24)
                  | ((uint32_t)entry[2] << 16)
                  | ((uint32_t)entry[1] << 8)
                  | ((uint32_t)entry[0])),
              "g"(pcb->esp0)
            : "esi"
        );
    }

    uint32_t ebp0 = pcb->ebp;
    tss.esp0 = pcb->esp0;
    tss.ss0 = KERNEL_DS;

    asm volatile (                                      /* flushes the TLB */
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        :::"eax"
    );
    
    pit_init(20);
    set_screen_coordinate(0, 0);
    
    asm volatile (
        "movl %0, %%ebp     \n"
        "leave              \n"
        "ret                \n"
        :
        : "r"(ebp0)
    );
}

void pit_handler() {
    send_eoi(0);

    get_screen_coordinate(
        &terms[active_term_id].cursor.x,
        &terms[active_term_id].cursor.y
    );                                              /* records the screen coordiante */
    
    uint32_t next_id = (active_term_id + 1) % TERMINAL_COUNT;
    if (next_id == shown_term_id) {                 /* setup paging for video memory*/
        page_table_kernel_vidmem[VIDMEM_INDEX].page_base_address = VIDMEM_INDEX;
        page_table_user_vidmem[VIDMEM_INDEX].page_base_address = VIDMEM_INDEX;
        set_cursor_pos(terms[next_id].cursor.x, terms[next_id].cursor.y);
    } else {
        page_table_kernel_vidmem[VIDMEM_INDEX].page_base_address = VIDMEM_INDEX + next_id + 2;
        page_table_user_vidmem[VIDMEM_INDEX].page_base_address = VIDMEM_INDEX + next_id + 2;
    }

    set_screen_coordinate(terms[next_id].cursor.x, terms[next_id].cursor.y);

    active_term_id = next_id;
    pcb_t *curr = get_current_pcb(), *next = pcbs[terms[next_id].pid];

    asm volatile (
        "movl %%ebp, %0\n"                          /* stores old ebp to return back */
        : "=g"(curr->ebp)
    );

    curr->esp0 = tss.esp0;
    tss.esp0 = next->esp0;
    page_directories[USER_ENTRY].MB.page_base_address = 2 + next->pid;
    page_table_user_vidmem[VIDMEM_INDEX].present = next->vidmap;

    asm volatile (                                  /* flushes the TLB */
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        :::"eax"
    );

    asm volatile (
        "movl %0, %%esp     \n"                     /* equivalent to leave, but one line less */
        "popl %%ebp         \n"
        "cmp $0, %1         \n"                     /* user pressed ctrl + C on this terminal */
        "jne to_be_halt     \n"
        "ret                \n"                     /* equivalent to set ebp, leave, ret */

        "to_be_halt:        \n"
        "pushl $6           \n"
        "call halt          \n"                     /* halt and reexecute */
        :
        : "g"(next->ebp),
          "g"(terms[next_id].input.to_be_halt)
        : "cc"
    );
}
