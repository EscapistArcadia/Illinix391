#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

const char *list[] = {
    "RegisterClassEx\t",
    "CreateWindowEx\t",
    "ShowWindow\t",
    "UpdateWindow\t",
    "GetCursorPos\t",
    "SetCursorPos\t",
    "GetWindowRect\t",
    "SetWindowRect\t",
    "SetWindowSubclass\t",
    "SetWindowLongPtr\t",
    "GetWindowLongPtr\t",
    "WaitForSingleObject\t"
};

int main() {
    int32_t fd = ece391_create((const uint8_t *)"written.txt");
    if (fd == -1) {
        ece391_fdputs(1, (const uint8_t *)"could not create file!");
        return 1;
    }

    int32_t written = 0, i;
    for (i = 0; i < 12; ++i) {
        written += ece391_write(fd, (const uint8_t *)list[i], ece391_strlen((const uint8_t *)list[i]));
        ece391_fdputs(1, (const uint8_t *)list[i]);
        ece391_fdputs(1, (const uint8_t *)" written\n");
    }

    ece391_close(fd);

    uint8_t buf[1024];
    fd = ece391_open((const uint8_t *)"written.txt");
    if (fd == -1) {
        return 1;
    }

    while ((i = ece391_read(fd, buf, 1024))) {
        if (-1 == i) {
            ece391_fdputs (1, (const uint8_t*)"file read failed\n");
            return 3;
        }
        
        if (-1 == ece391_write (1, buf, i)){
            ece391_fdputs(1, (const uint8_t *)"failed to write on terminal");
            return 3;
        }
    }

    ece391_close(fd);

    return 0;
}