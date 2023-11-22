int pid_to_pname(char *pid, char *buf) {
    if (strspn(pid, "0123456789") != strlen(pid)) {
        return 0;
    }
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "/proc/%s/stat", pid);
    FILE* f = fopen(tmp, "r");
    if (f == NULL) return 0;
    if (fgets(tmp, sizeof(tmp), f) == NULL) {
        fclose(f);
        return 0;
    }
    fclose(f);
    int u;
    sscanf(tmp, "%d (%[^)]s", &u, buf);
    return 1;
}