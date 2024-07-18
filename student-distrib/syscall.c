#include "syscall.h"
#include "paging.h"
#include "term.h"
#include "rtc.h"
#include "filesys.h"

pcb_t *pcbs[MAX_PROCESS] = {
    (pcb_t *)(KERNEL_STACK - (0x00 + 1) * KERNEL_STACK_SIZE),
    (pcb_t *)(KERNEL_STACK - (0x01 + 1) * KERNEL_STACK_SIZE),
    (pcb_t *)(KERNEL_STACK - (0x02 + 1) * KERNEL_STACK_SIZE),
};

int32_t null_open(const uint8_t *file_name) {return -1;}
int32_t null_read(int32_t fd, void *buf, uint32_t count) {return -1;}
int32_t null_write(int32_t fd, const void *buf, uint32_t count) {return -1;}
int32_t null_close(int32_t fd) {return -1;}

file_operations_t null_ops = {
    .open = null_open,
    .close = null_close,
    .read = null_read,
    .write = null_write
};

file_operations_t stdin_ops = {
    .open = terminal_open,
    .close = terminal_close,
    .read = terminal_read,
    .write = null_write
};

file_operations_t stdout_ops = {
    .open = terminal_open,
    .close = terminal_close,
    .read = null_read,
    .write = terminal_write
};

file_operations_t file_ops = {
    .open = file_open,
    .close = file_close,
    .read = file_read,
    .write = file_write,
};

file_operations_t dir_ops = {
    .open = dir_open,
    .close = dir_close,
    .read = dir_read,
    .write = dir_write
};

file_operations_t rtc_ops = {
    .open = rtc_open,
    .close = rtc_close,
    .read = rtc_read,
    .write = rtc_write
};

/**
 * @brief terminates the currently executing user program, with exit code \p status
 * 
 * @param status the status code for the kernel
 * @return 0 if success, -1 if fail
 */
int32_t halt(uint8_t status) {
    int i;
    pcb_t *pcb = get_current_pcb();
    
    if (pcb->pid == 0) {                                /* never closes the terminal */
        return 0;
    }

    /* *************** Reclaim the PCB & Resources *************** */
    pcb->present = 0;
    for (i = 0; i < 8; ++i) {
        if (pcb->files[i].present) {
            pcb->files[i].ops->close(i);                /* closes all files */
            pcb->files[i].present = 0;                  /* reclaims all resources*/
        }
    }

    /* *************** Restore Paging For Parent *************** */
    page_directories[USER_ENTRY].MB.page_base_address = 2 + pcb->parent->pid;
    asm volatile (                                      /* flushes the TLB */
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        :::"eax"
    );

    tss.esp0 = pcb->parent->esp0;
    tss.ss0 = KERNEL_DS;

    extern uint8_t exception_occurred;
    if (exception_occurred) {
        exception_occurred = 0;                             /* return 256; */
        asm volatile (
            "movl $0x100, %%eax \n"                         /* assignes the return value */
            "movl %0, %%ebp     \n"                         /* set the context to execute() */
            // "leave\n"                                    /* %esp = %ebp = pcb->parent_ebp, popl %ebp */
            // "ret\n"                                      /* popl %eip */
            :
            : "r"(pcb->parent_ebp)
            : "%eax"
        );
    } else {
        asm volatile (
            "movl %0, %%eax\n"                              /* assignes the return value */
            "movl %1, %%ebp\n"                              /* set the context to execute() */
            :
            : "r"((uint32_t)status), "r"(pcb->parent_ebp)
            : "%eax"
        );
    }
 
    asm volatile (
        "leave  \n"
        "ret    \n"
    );

    return 0xECE391;
}

/**
 * @brief runs a user program with parameter(s) specified in \p command
 * 
 * @param command user-input command
 * @return 0 if success, -1 if fail
 */
int32_t execute(const uint8_t *command) {
    if (command == NULL) {
        return -1;
    }

    /* *************** Parse Command *************** */
    int i = 0;
    uint8_t file_name[MAX_TERMINAL] = {0};              /* the file name (first part) */
    uint8_t argument[MAX_TERMINAL] = {0};               /* the added argument (nullable, second) */

    const uint8_t *pos = command;
    uint8_t *argv_pos;
    for (; *pos == ' ' && !(i & ~(MAX_TERMINAL - 1)); ++i, ++pos);
    if (!*pos) {                                        /* skips spaces before command */
        return -1;                                      /* no command? */
    }
    for (argv_pos = file_name; *pos && *pos != ' ' && !(i & ~(MAX_TERMINAL - 1)); ++i, ++pos, ++argv_pos) {
        *argv_pos = *pos;                               /* parses command */
    }
    for (; *pos == ' ' && !(i & ~(MAX_TERMINAL - 1)); ++i, ++pos);  /* skips spaces between command and arguments*/
    if (*pos) {                                         /* parses arguments if has*/
        for (argv_pos = argument; *pos && *pos != ' ' && !(i & ~(MAX_TERMINAL - 1)); ++i, ++pos, ++argv_pos) {
            *argv_pos = *pos;
        }
        *argv_pos = 0;
    }

    /* *************** Check Excutability *************** */
    uint32_t magic;
    dentry_t den;
    if ((read_dentry_by_name(file_name, &den) == -1)    /* checks the existence of the file */
        || (read_data(den.inode_num, 0, (uint8_t *)&magic, sizeof(uint32_t)) == -1)
        || magic != EXECUTABLE_MAGIC) {                 /* checks if the file is executable */
        return -1;
    }

    /* *************** Check Availablility *************** */
    for (i = 0; i < MAX_PROCESS && pcbs[i]->present != 0; ++i);
    if (i == MAX_PROCESS) {                             /* check available pcb address */
        return -1;
    }
    
    /* *************** Set Up PCB *************** */
    int32_t pid = i;
    pcb_t *pcb = pcbs[pid];

    pcb->present = 1;
    pcb->pid = pid;
    pcb->parent = pid == 0 ? NULL : get_current_pcb();  /* pid = 0 => terminal */
    asm volatile (
        "movl %%ebp, %0"                                /* records the return address */
        : "=g"(pcb->parent_ebp)
    );
    pcb->esp0 = KERNEL_STACK - KERNEL_STACK_SIZE * pid;
    memcpy(pcb->argv, argument, argv_pos - argument + 1);
    
    pcb->files[0].present = 1;
    pcb->files[0].ops = &stdin_ops;
    pcb->files[1].present = 1;
    pcb->files[1].ops = &stdout_ops;
    for (i = 2; i < 8; ++i) {
        pcb->files[i].present = 0;
    }

    /* *************** Set Up Paging *************** */
    page_directories[USER_ENTRY].MB.present = 1;
    page_directories[USER_ENTRY].MB.user_supervisor = 1;/* user can access the page */
    page_directories[USER_ENTRY].MB.read_write = 1;     /* user can write the page */
    page_directories[USER_ENTRY].MB.page_base_address = 2 + pid;
    asm volatile (                                      /* flushes the TLB */
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        :::"eax"
    );

    uint8_t entry[4];
    read_data(den.inode_num, 24, (uint8_t *)entry, sizeof(entry));  /* gets the entry */
    read_data(den.inode_num, 0, (uint8_t *)PROGRAM_IMAGE, PROGRAM_IMAGE_LIMIT);

    tss.esp0 = pcb->esp0;
    tss.ss0 = KERNEL_DS;

    asm volatile (                                      /* setup user-level iret context */
        "movw %w0, %%ds\n"
        "pushl %0\n"                                    /* SS */
        "pushl %1\n"                                    /* ESP */
        "sti\n"
        "pushfl\n"                                      /* EFLAGS */
        "pushl %2\n"                                    /* CS */
        "pushl %3\n"                                    /* EIP */
        "iret\n"                                        /* pops and start executing user codes */
        :
        : "r"((uint32_t)USER_DS),
          "g"((uint32_t)USER_STACK),
          "g"((uint32_t)USER_CS),
          "r"(((uint32_t)entry[3] << 24)                /* reverses the entry */
              | ((uint32_t)entry[2] << 16)
              | ((uint32_t)entry[1] << 8)
              | ((uint32_t)entry[0]))
        : "memory"
    );

    return 0xECEB3026;                                  /* never reaches here */
}

/**
 * @brief continues to read a file from the position last time, or
 * 0 for the first time
 * 
 * @param fd the file descriptor of the file to read
 * @param buf the destination buffer of file content
 * @param count the buffer's capacity
 * @return int32_t count of bytes read
 */
int32_t read(int32_t fd, void* buf, int32_t count) {
    pcb_t *curr = get_current_pcb();
    if (fd < 0 || fd >= 8 || curr->files[fd].present == 0) {
        return -1;
    }

    int32_t read_bytes = curr->files[fd].ops->read(fd, buf, count);
    if (read_bytes >= 0) {
        curr->files[fd].file_pos += read_bytes;                 /* to continue read */
        return read_bytes;
    }
    return -1;
}

/**
 * @brief continues to write a file from the position last time, or
 * 0 for the first time
 * 
 * @param fd the file descriptor of the file to read
 * @param buf the source buffer of content to write
 * @param count count of bytes intended to write
 * @return count of bytes wrote
 */
int32_t write(int32_t fd, const void* buf, int32_t count) { 
    pcb_t *curr = get_current_pcb();
    if (fd < 0 || fd >= 8 || curr->files[fd].present == 0) {
        return -1;
    }
    
    uint32_t written_bytes = curr->files[fd].ops->write(fd, buf, count);
    if (written_bytes >= 0) {
        curr->files[fd].file_pos += written_bytes;
        return written_bytes;
    }
    return -1;
}

/**
 * @brief opens a file from the file system
 * 
 * @param file_name name of the file
 * @return int32_t the file descriptor to the process
 */
int32_t open(const uint8_t* file_name) {
    static file_operations_t *const ops_map[] = {
        &rtc_ops,
        &dir_ops,
        &file_ops
    };

    pcb_t *curr = get_current_pcb();

    int i;
    for (i = 2; i < 8; ++i) {
        if (!curr->files[i].present) {
            dentry_t den;
            if (read_dentry_by_name(file_name, &den) == -1) {       /* checks the existence & file type */
                return -1;
            }

            if (ops_map[den.file_type]->open(file_name) == -1) {
                return -1;
            }

            /* set up file structure */
            curr->files[i].ops = ops_map[den.file_type];
            curr->files[i].inode = den.inode_num;
            curr->files[i].file_pos = 0;
            curr->files[i].present = 1;
            return i;
        }
    }
    return -1;
}

/**
 * @brief closes the opened file and releases the descriptor
 * 
 * @param fd the file descriptor
 * @return 0 if success, -1 if fail
 */
int32_t close(int32_t fd) {
    pcb_t *curr = get_current_pcb();
    if (fd < 2 || fd >= 8 || curr->files[fd].present == 0) {
        return -1;
    }
    
    if (curr->files[fd].ops->close(fd) == 0) {
        return (curr->files[fd].present = 0);
    }
    return -1;
}

/**
 * @brief gets the arguments user passed to the program as the
 * program started
 * 
 * @param buf the destination buffer address
 * @param nbytes the capacity of the buffer
 * @return number of bytes read
 */
int32_t getargs(uint8_t* buf, int32_t count) {
    if (buf == NULL || ((uint32_t)buf >> 22) != USER_ENTRY) {
        return -1;
    }
    
    int i;
    uint8_t *src_pos = get_current_pcb()->argv, *dest_pos = buf;
    for (i = 0; *src_pos && i < count; ++i, ++src_pos, ++dest_pos) {
        *dest_pos = *src_pos;
    }
    *dest_pos = (uint8_t)0;
    return 0;
}

/**
 * @brief requested the kernel for writing content on the video
 * memory
 * 
 * @param start the address to write the address of video memory
 * entry
 * @return 0 if success, -1 if fails 
 */
int32_t vidmap(uint8_t** start) {
    return 0;
}

int32_t set_handler(int32_t signum, void *handler_address) { return 0; }

int32_t sigreturn(void) { return 0; }
