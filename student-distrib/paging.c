#include "paging.h"
#include "syscall.h"

#define PAGING_FLAG  0x80000001 /* first: paging enable; last: protection mode*/
#define PAGING_SIZE_EXTENTION_FLAG 0x00000010 /* enables 4MB pages */

void paging_init() {
    memset(page_directories, 0, sizeof(page_directories));
    memset(page_table_kernel_vidmem, 0, sizeof(page_table_kernel_vidmem));
    memset(page_table_user_vidmem, 0, sizeof(page_table_user_vidmem));

    int i;
    page_directories[0].KB.present = 1;
    page_directories[0].val |= (uint32_t)page_table_kernel_vidmem;
    page_directories[1].MB.present = 1;
    page_directories[1].MB.page_size = 1;
    page_directories[1].val |= KERNEL_ADDR;
    page_table_kernel_vidmem[1].page_base_address = 1;
    page_table_user_vidmem[1].page_base_address = 1;
    for (i = 2; i < PAGING_COUNT; ++i) {
        page_directories[i].MB.page_size = 1;
        page_directories[i].MB.page_base_address = i;
        page_table_kernel_vidmem[i].page_base_address = i;
        page_table_user_vidmem[i].page_base_address = i;
    }
    page_table_kernel_vidmem[VIDMEM_INDEX].present = 1;
    page_table_user_vidmem[VIDMEM_INDEX].present = 1;
    page_table_user_vidmem[VIDMEM_INDEX].user_supervisor = 1;
    page_table_user_vidmem[VIDMEM_INDEX].read_write = 1;

    page_directories[VIDMEM_INDEX].KB.present = 1;
    page_directories[VIDMEM_INDEX].KB.user_supervisor = 1;
    page_directories[VIDMEM_INDEX].KB.read_write = 1;
    page_directories[VIDMEM_INDEX].KB.page_size = 0;
    page_directories[VIDMEM_INDEX].KB.page_table_base_address = ((uint32_t)page_table_user_vidmem >> 12);

    asm volatile (
        "movl %0, %%cr3\n"
        "movl %%cr4, %%edx\n"
        "orl  %1, %%edx\n"
        "movl %%edx, %%cr4\n"
        "movl %%cr0, %%edx\n"
        "orl  %2, %%edx\n"
        "movl %%edx, %%cr0\n"
        :
        :"r"(page_directories), "r"(PAGING_SIZE_EXTENTION_FLAG), "r"(PAGING_FLAG)
        :"%edx"
    );
}
