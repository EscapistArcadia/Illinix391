#include "malloc.h"
#include "paging.h"

#define PAGE_FRAME_SIZE 0x1000
#define KMALLOC_BEGIN   0x3000000
#define KMALLOC_PDE     (KMALLOC_BEGIN >> 22)
#define KMALLOC_PDE_COUNT   4
#define KMALLOC_SIZE    (PAGE_FRAME_SIZE * PAGING_COUNT * KMALLOC_PDE_COUNT)

#define ALIGNED_SIZE(x) ((x + 15) & (~15))

pte_t page_table_kmalloc[PAGING_COUNT * KMALLOC_PDE_COUNT] __attribute__((aligned(PAGING_ALIGN)));

typedef struct block_t {
    struct block_t *prev;
    struct block_t *next;
    void *start;
    uint32_t size;
} block_t;

block_t *kmem_blocks;

void kmalloc_init() {
    pte_t *pos = page_table_kmalloc;
    uint32_t i, j = 0;
    for (i = 0; i < KMALLOC_PDE_COUNT; ++i) {
        page_directories[KMALLOC_PDE + i].KB.present = 1;
        page_directories[KMALLOC_PDE + i].KB.user_supervisor = 0;
        page_directories[KMALLOC_PDE + i].KB.read_write = 1;
        page_directories[KMALLOC_PDE + i].KB.page_size = 0;
        page_directories[KMALLOC_PDE + i].KB.page_table_base_address = ((uint32_t)(pos) >> 12);

        for (j = 0; j < PAGING_COUNT; ++j, ++pos) {
            pos->present = 1;
            pos->user_supervisor = 0;
            pos->read_write = 1;
            pos->page_base_address = j;
        }
    }
    memset((void *)KMALLOC_BEGIN, 0, KMALLOC_SIZE);
    // kmem_blocks->prev = kmem_blocks->next = NULL;
    // kmem_blocks->start = (void *)(KMALLOC_BEGIN + sizeof(block_t));
    // kmem_blocks->size = KMALLOC_SIZE - sizeof(block_t);
}

// static uint32_t block_size[] = {
//     sizeof(block_t) << 0,
//     sizeof(block_t) << 1,
//     sizeof(block_t) << 2,
//     sizeof(block_t) << 3,
//     sizeof(block_t) << 4,
//     sizeof(block_t) << 5,
//     sizeof(block_t) << 6,
//     sizeof(block_t) << 7,
//     sizeof(block_t) << 8,
//     sizeof(block_t) << 9,
//     sizeof(block_t) << 10,
//     sizeof(block_t) << 11,
//     sizeof(block_t) << 12,
//     sizeof(block_t) << 13,
//     sizeof(block_t) << 14,
//     sizeof(block_t) << 15,
// };

/**
 * @brief allocates a block of memory in \p size for kernel-level
 * program
 * 
 * @param size size of memory in bytes
 * @return starting address of memory
 */
void *kmalloc(uint32_t size) {
    if (!size || size > 0x400000) {             /* kmalloc is for small block of data */
        return NULL;
    }
    
    size = ALIGNED_SIZE(size);
    uint32_t real_size = size + sizeof(block_t);
    block_t *curr;
    for (curr = kmem_blocks; curr; curr = curr->next) {
        if (curr->size >= size) {
            if (curr->size - size < sizeof(block_t)) {
                if (curr->prev) {               /* remove this block from free blocks */
                    curr->prev->next = curr->next;
                }
                if (curr->next) {
                    curr->next->prev = curr->prev;
                }
                return (void *)(curr + 1);      /* all spaces is now used up */
            }

            curr->size = size;
            block_t *next = (block_t *)((unsigned char *)curr + real_size);
            next->size = curr->size - real_size;
            next->prev = curr->prev;
            next->next = curr->next;
            if (curr->prev) {
                curr->prev->next = next;
            }
            if (curr->next) {
                curr->next->prev = next;
            }
            if (curr == kmem_blocks) {
                kmem_blocks = next;
            }
            return (void *)(curr + 1);
        }
    }

    kmem_blocks = (block_t *)(KMALLOC_BEGIN);  /* no alloc happens */
    kmem_blocks->size = size;                  /* the remaining size */
    kmem_blocks->prev = NULL;                  /* the first one */
    kmem_blocks->next = NULL;
    kmem_blocks = (block_t *)((unsigned char*)kmem_blocks + real_size);
    kmem_blocks->size = (KMALLOC_SIZE - real_size);
    kmem_blocks->prev = NULL;
    kmem_blocks->next = NULL;
    return (void *)(KMALLOC_BEGIN + sizeof(block_t));
}
