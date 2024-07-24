#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main() {
    uint8_t buf[128];
    if (ece391_getargs(buf, 128) == -1) {
        ece391_fdputs(1, (const uint8_t *)"could not read arguments!");
        return 2;
    }

    if (ece391_delete(buf) == -1) {
        ece391_fdputs(1, (const uint8_t *)"failed to delete");
        return 3;
    }
    return 0;
}
