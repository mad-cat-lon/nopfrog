#include <string.h> 
#include <stdio.h> 

int check_if_hidden_path(const char *path) {
    char *hidden_str = strdup(HIDDEN_STR); xor(hidden_str);
    if ((strstr(path, hidden_str)) != NULL) {
        CLEAN(hidden_str);
        return 1;
    }
    CLEAN(hidden_str);
    return 0;
}