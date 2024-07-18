#ifndef _PAGING_H
#define _PAGING_H

#include "lib.h"

#define PAGING_COUNT 1024
#define PAGING_ALIGN 0x1000

#define VIDMEM_INDEX 0xB8

#define KERNEL_ADDR  0x400000
#define KERNEL_INDEX (KERNEL_ADDR >> 12)

pde_t page_directories[PAGING_COUNT] __attribute__((aligned(PAGING_ALIGN)));
pte_t page_table_kernel_vidmem[PAGING_COUNT] __attribute__((aligned(PAGING_ALIGN)));

void paging_init();

#endif
