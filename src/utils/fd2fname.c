#include <string.h> 
#include <stdio.h> 
#include "../rkconsts.h"

int fd_to_fname(int fd, char *buf, size_t size) {
    char proc_fd_path[256];
    pid_t pid = getpid();
    snprintf(proc_fd_path, 256, "/proc/%lu/fd/%d", (long)pid, fd);
    ssize_t ret = readlink(proc_fd_path, buf, size);
    if (ret == -1) {
        return 0;
    }
    else {
        buf[ret] = 0;
        return 1;
    }
}