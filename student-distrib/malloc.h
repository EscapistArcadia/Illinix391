#ifndef _MALLOC_H
#define _MALLOC_H

#include "lib.h"

/**
 * @brief allocates a block of runtime memory with size \p size
 * 
 * @param size size of memory in byte
 * @return allocated memory
 */
extern void *malloc(uint32_t size);

/**
 * @brief enlarges the allocated memory starting at \p src with new
 * size \p size
 * 
 * @param src starting address of memory
 * @param size new size of memory in byte
 * @return allocated memory
 */
extern void *realloc(void *src, uint32_t size);

/**
 * @brief releases the ownership of the memory starting at \p src
 * 
 * @param src starting address of memory
 */
extern void free(void *src);

/**
 * @brief initializes memory and page table entries for kmalloc
 */
extern void kmalloc_init();

/**
 * @brief allocates a block of memory in \p size for kernel-level
 * program
 * 
 * @param size size of memory in bytes
 * @return starting address of memory
 */
extern void *kmalloc(uint32_t size);

extern void *get_free_pages(uint32_t order);

#endif
